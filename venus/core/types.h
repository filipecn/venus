/// Copyright (c) 2025, FilipeCN.
///
/// The MIT License (MIT)
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///
///\file types.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2025-06-07
///
///\brief

#pragma once

#include <cstdint>

using f32 = float;  //!< 32 bit size floating point type
using f64 = double; //!< 64 bit size floating point type

using i8 = int8_t;   //!<  8 bit size integer type
using i16 = int16_t; //!< 16 bit size integer type
using i32 = int32_t; //!< 32 bit size integer type
using i64 = int64_t; //!< 64 bit size integer type

using u8 = uint8_t;   //!<  8 bit size unsigned integer type
using u16 = uint16_t; //!< 16 bit size unsigned integer type
using u32 = uint32_t; //!< 32 bit size unsigned integer type
using u64 = uint64_t; //!< 64 bit size unsigned integer type

using ulong = unsigned long;   //!< unsigned long type
using uint = unsigned int;     //!< unsigned int type
using ushort = unsigned short; //!< unsigned short type
using uchar = unsigned char;   //!< unsigned char type

using byte = uint8_t; //!< unsigned byte

namespace venus::core {

enum class VenusResult {
  SUCCESS = 0,
  BAD_ALLOCATION = 1,
  OUT_OF_BOUNDS = 2,
  INVALID_INPUT = 3,
  BAD_OPERATION = 4
};

}
