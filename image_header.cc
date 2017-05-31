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

#include "image_header.h"

#include "endian.h"

using std::vector;

namespace tools {

struct RawImageHeader {
  uint64_t start_tag;
  ImageSignature image_signature;
  uint32_t fiu0_drd_cfg;
  uint8_t fiu_clk_divider;
  std::array<uint8_t, 51> reserved;
  uint32_t dest_addr;
  uint32_t code_size;
  uint32_t version;
  std::array<uint8_t, 180> reserved_signed;
} ATTRIBUTE_PACKED;

const size_t ImageHeader::kHeaderSize = sizeof(RawImageHeader);

void ImageHeader::ToBuffer(vector<uint8_t>* buffer) const {
  buffer->resize(sizeof(RawImageHeader));
  RawImageHeader* image_header = reinterpret_cast<RawImageHeader*>(buffer->data());
  image_header->start_tag = LittleEndian::FromHost64(start_tag_);
  image_header->image_signature = image_signature_;
  image_header->fiu0_drd_cfg = LittleEndian::FromHost32(fiu0_drd_cfg_);
  image_header->fiu_clk_divider = fiu_clk_divider_;
  image_header->reserved.fill(0xff);
  image_header->dest_addr = LittleEndian::FromHost32(dest_addr_);
  image_header->code_size = LittleEndian::FromHost32(code_size_);
  image_header->version = LittleEndian::FromHost32(version_);
  image_header->reserved_signed.fill(0xff);
}

int ImageHeader::FromBuffer(const vector<uint8_t>& buffer) {
  if (buffer.size() != sizeof(RawImageHeader)) {
    return -EINVAL;
  }
  const RawImageHeader& image_header = *reinterpret_cast<const RawImageHeader*>(buffer.data());
  start_tag_ = LittleEndian::ToHost64(image_header.start_tag);
  image_signature_ = image_header.image_signature;
  fiu0_drd_cfg_ = LittleEndian::ToHost32(image_header.fiu0_drd_cfg);
  fiu_clk_divider_ = image_header.fiu_clk_divider;
  dest_addr_ = LittleEndian::ToHost32(image_header.dest_addr);
  code_size_ = LittleEndian::ToHost32(image_header.code_size);
  version_ = LittleEndian::ToHost32(image_header.version);
  return 0;
}

}  // namespace tools
