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

#ifndef IMAGE_HEADER_H_
#define IMAGE_HEADER_H_

#include <array>
#include <vector>
#include <cstring>

#include <stdint.h>

namespace tools {

typedef std::array<uint8_t, 256> ImageSignature;

class ImageHeader {
 public:
  static const size_t kHeaderSize;

  ImageHeader() { image_signature_.fill(0xff); }

  uint64_t start_tag() const { return start_tag_; }
  const ImageSignature& image_signature() const { return image_signature_; }
  uint32_t fiu0_drd_cfg() const { return fiu0_drd_cfg_; }
  uint8_t fiu_clk_divider() const { return fiu_clk_divider_; }
  uint8_t* reserved_0() { return reserved_0_; }
  uint64_t boot_block_magic() const { return boot_block_magic_; }
  uint8_t* reserved_1() { return reserved_1_; }
  uint32_t dest_addr() const { return dest_addr_; }
  uint32_t code_size() const { return code_size_; }
  uint32_t version() const { return version_; }

  void ToBuffer(std::vector<uint8_t>* buffer) const;

  void set_start_tag(uint64_t start_tag) { start_tag_ = start_tag; }
  void set_image_signature(const ImageSignature& image_signature) { image_signature_ = image_signature; }
  void set_fiu0_drd_cfg(uint32_t fiu0_drd_cfg) { fiu0_drd_cfg_ = fiu0_drd_cfg; }
  void set_fiu_clk_divider(uint8_t fiu_clk_divider) { fiu_clk_divider_ = fiu_clk_divider; }
  void set_reserved_0(uint8_t reserved_0[]) { memcpy(reserved_0_, reserved_0, sizeof(reserved_0_)); }
  void set_boot_block_magic(uint64_t boot_block_magic) { boot_block_magic_ = boot_block_magic; }
  void set_reserved_1(uint8_t reserved_1[]) { memcpy(reserved_1_, reserved_1, sizeof(reserved_1_)); }
  void set_dest_addr(uint32_t dest_addr) { dest_addr_ = dest_addr; }
  void set_code_size(uint32_t code_size) { code_size_ = code_size; }
  void set_version(uint32_t version) { version_ = version; }

  int FromBuffer(const std::vector<uint8_t>& buffer);

 private:
  uint64_t start_tag_ = 0;
  ImageSignature image_signature_;
  uint32_t fiu0_drd_cfg_ = 0;
  uint8_t fiu_clk_divider_ = 0;
  uint8_t reserved_0_[19];
  uint64_t boot_block_magic_ = 0xffffffffffffffff;
  uint8_t reserved_1_[24];
  uint32_t dest_addr_ = 0;
  uint32_t code_size_ = 0;
  uint32_t version_ = 0;
};

}  // namespace tools

#endif  // IMAGE_HEADER_H_
