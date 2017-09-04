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

#ifndef ENDIAN_H_
#define ENDIAN_H_

#include <stdint.h>

#if defined(__i386__) || defined(__x86_64__)
#define LE
#endif

class LittleEndian {
 public:
#ifdef LE
  static uint16_t FromHost16(uint16_t value) { return value; }
  static uint32_t FromHost32(uint32_t value) { return value; }
  static uint64_t FromHost64(uint64_t value) { return value; }
  static uint16_t ToHost16(uint16_t value) { return value; }
  static uint32_t ToHost32(uint32_t value) { return value; }
  static uint64_t ToHost64(uint64_t value) { return value; }
#else // LE
#error "Functions not defined for host's endianness."
#endif
};

#endif  // ENDIAN_H_
