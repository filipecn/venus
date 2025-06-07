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
