// Copyright 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "image_header.h"

using std::string;
using std::vector;

namespace tools {
namespace {

class FileCloser {
 public:
  FileCloser(int fd) : fd_(fd) {}

  ~FileCloser() {
    // TODO: We should log whether close fails.
    close(fd_);
  }

 private:
  int fd_ = 0;
};

int GetFileContents(const string& file_name, vector<uint8_t>* buffer) {
  int fd = open(file_name.c_str(), O_RDONLY);
  if (fd < 0) {
    return -errno;
  }
  FileCloser closer(fd);

  struct stat stat;
  int ret = fstat(fd, &stat);
  if (ret < 0) {
    return -errno;
  }
  buffer->resize(stat.st_size);

  ssize_t amount_read = read(fd, buffer->data(), buffer->size());
  if (amount_read < 0) {
    return -errno;
  }
  if (amount_read < buffer->size()) {
    return -EIO;
  }
  return 0;
}

int SetFileContents(const string& file_name, const vector<uint8_t>& buffer) {
  int fd = open(file_name.c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
  if (fd < 0) {
    return -errno;
  }
  FileCloser closer(fd);

  ssize_t amount_written = write(fd, buffer.data(), buffer.size());
  if (amount_written < 0) {
    return -errno;
  }
  if (amount_written < buffer.size()) {
    return -EIO;
  }
  return 0;
}

} // namespace

class ImageBuilder {
 public:
  virtual ImageHeader ConstructImageSpecificHeader() = 0;

  virtual int CreateImage(const string& binary_name,
                          const string& image_name,
                          uint32_t fiu0_drd_cfg,
                          uint8_t fiu_clk_divider,
                          uint16_t mc_freq,
                          uint16_t cpu_freq,
                          uint8_t mc_cfg,
                          uint32_t version) {
    vector<uint8_t> image;
    int ret = GetFileContents(binary_name, &image);
    if (ret < 0) {
      return ret;
    }

    ImageHeader header = ConstructImageSpecificHeader();
    // TODO: Add support for image signature.
    header.set_fiu0_drd_cfg(fiu0_drd_cfg);
    header.set_fiu_clk_divider(fiu_clk_divider);
    header.set_mc_freq(mc_freq);
    header.set_cpu_freq(cpu_freq);
    header.set_mc_cfg(mc_cfg);
    header.set_code_size(image.size());
    header.set_version(version);
    vector<uint8_t> raw_header;
    header.ToBuffer(&raw_header);

    image.insert(image.begin(), raw_header.begin(), raw_header.end());
    return SetFileContents(image_name, image);
  }
};

class BootBlockImageBuilder : public ImageBuilder {
 public:
  // BOOT.U.P or "P.U.TOOB" in Little Endian
  static const uint64_t kBootBlockStartTag = 0x424f4f54aa550750;

  ImageHeader ConstructImageSpecificHeader() override {
    ImageHeader header;
    header.set_start_tag(kBootBlockStartTag);
    // TODO: Should this be more configurable?
    header.set_boot_block_magic(0x0000000100000002);
    header.set_dest_addr(0xfffd5e00);
    return header;
  }
};

class UbootImageBuilder : public ImageBuilder {
 public:
  // KLBTOOBU or "UBOOTBLK" in Little Endian
  static const uint64_t kUbootStartTag = 0x4b4c42544f4f4255;

  ImageHeader ConstructImageSpecificHeader() override {
    ImageHeader header;
    header.set_start_tag(kUbootStartTag);
    header.set_dest_addr(0x00008000);
    return header;
  }
};

}  // namespace tools

using tools::BootBlockImageBuilder;
using tools::ImageBuilder;
using tools::UbootImageBuilder;

struct Flags {
  uint32_t fiu0_drd_cfg = 0x030111bc;
  uint8_t fiu_clk_divider = 0x00;
  uint16_t mc_freq = 800;
  uint16_t cpu_freq = 800;
  // TODO: Split out bits into bool flags
  uint8_t mc_cfg = 0x1;
};

bool SplitString(const string& input, const string& delimiter, string* left, string* right) {
  string::size_type pos = input.find(delimiter);
  if (pos == string::npos) {
    return false;
  }

  *left = input.substr(0, pos);
  *right = input.substr(pos + delimiter.size());
  return true;
}

bool ParseUint64Flag(const string& input,
                     const string& expected_name,
                     int base,
                     uint64_t* value) {
  string flag_name;
  string flag_value;
  if (!SplitString(input, "=", &flag_name, &flag_value)) {
    return false;
  }

  if (flag_name != expected_name) {
    return false;
  }

  try {
    *value = stoull(flag_value, nullptr, base);
  } catch (const std::exception& e) {
    std::cout << "failed to parse integer: " << e.what();
    return false;
  }
  return true;
}

bool ParseUint32Flag(char* argv[],
                     int* index,
                     const string& expected_name,
                     int base,
                     uint32_t* value) {
  uint64_t parsed_value;
  if (ParseUint64Flag(argv[*index], expected_name, base, &parsed_value) &&
      parsed_value < UINT32_MAX) {
    *value = parsed_value;
    (*index)++;
    return true;
  }
  return false;
}

bool ParseUint16Flag(char* argv[],
                     int* index,
                     const string& expected_name,
                     int base,
                     uint16_t* value) {
  uint64_t parsed_value;
  if (ParseUint64Flag(argv[*index], expected_name, base, &parsed_value) &&
      parsed_value < UINT16_MAX) {
    *value = parsed_value;
    (*index)++;
    return true;
  }
  return false;
}

bool ParseUint8Flag(char* argv[],
                    int* index,
                    const string& expected_name,
                    int base,
                    uint8_t* value) {
  uint64_t parsed_value;
  if (ParseUint64Flag(argv[*index], expected_name, base, &parsed_value) &&
      parsed_value < UINT8_MAX) {
    *value = parsed_value;
    (*index)++;
    return true;
  }
  return false;
}

bool ParseFiu0DrdCfg(char* argv[], int* index, Flags* flags) {
  return ParseUint32Flag(argv, index, "--fiu0_drd_cfg", 16,
                         &flags->fiu0_drd_cfg);
}

bool ParseFiuClkDivider(char* argv[], int* index, Flags* flags) {
  return ParseUint8Flag(argv, index, "--fiu_clk_divider", 16,
                        &flags->fiu_clk_divider);
}

// TODO: Check against the list of acceptable frequencies
bool ParseMcFreq(char* argv[], int* index, Flags* flags) {
  return ParseUint16Flag(argv, index, "--mc_freq", 10, &flags->mc_freq);
}

bool ParseCpuFreq(char* argv[], int* index, Flags* flags) {
  return ParseUint16Flag(argv, index, "--cpu_freq", 10, &flags->cpu_freq);
}

bool ParseMcCfg(char* argv[], int* index, Flags* flags) {
  return ParseUint8Flag(argv, index, "--mc_cfg", 16, &flags->mc_cfg);
}

Flags ParseFlags(char* argv[], int* index, int argc) {
  vector<std::function<bool(char*[], int*, Flags*)>> parsers = {
    ParseFiu0DrdCfg,
    ParseFiuClkDivider,
    ParseMcFreq,
    ParseCpuFreq,
    ParseMcCfg,
  };
  Flags flags;
  bool flag_parsed = true;
  while (*index + 2 < argc && flag_parsed) {
    flag_parsed = false;
    for (std::function<bool(char*[], int*, Flags*)> parser : parsers) {
      if (parser(argv, index, &flags)) {
        flag_parsed = true;
        break;
      }
    }
  }
  return flags;
}

int main(int argc, char* argv[]) {
  if (argc < 4) {
    std::cout << "Must at least provide: binary type, image-in name,"
              << " and image-out name" << std::endl;
    return 1;
  }

  const std::string binary_type(argv[1]);
  std::unique_ptr<ImageBuilder> image_builder;
  uint32_t version = 0;
  if (binary_type == "--bootblock") {
    // TODO: We should probably make the version a command line option.
    version = 0x00000201;
    image_builder = std::unique_ptr<ImageBuilder>(new BootBlockImageBuilder());
  } else if (binary_type == "--uboot") {
    image_builder = std::unique_ptr<ImageBuilder>(new UbootImageBuilder());
  } else {
    std::cout << "invalid binary type: " << binary_type << std::endl;
    return 1;
  }

  int argv_index = 2;

  Flags flags = ParseFlags(argv, &argv_index, argc);
  if (argv_index + 2 != argc) {
    std::cout << "Too many or illegal arguments provided: ";
    for (; argv_index < argc; argv_index++) {
      std::cout << argv[argv_index] << " ";
    }
    std::cout << std::endl;
    return 1;
  }

  const std::string binary_name(argv[argv_index++]);
  const std::string image_name(argv[argv_index++]);
  int ret = image_builder->CreateImage(binary_name,
                                       image_name,
                                       flags.fiu0_drd_cfg,
                                       flags.fiu_clk_divider,
                                       flags.mc_freq,
                                       flags.cpu_freq,
                                       flags.mc_cfg,
                                       version);
  if (ret < 0) {
    std::cout << "error occurred when creating image: "
              << strerror(-ret) << std::endl;
    return 1;
  }

  return 0;
}
