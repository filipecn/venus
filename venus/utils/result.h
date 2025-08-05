/* Copyright (c) 2025, FilipeCN.
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/// \file   result.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-06-07
/// \brief  Venus return object.

#include <hermes/core/result.h>

#define VENUS_DEBUG

#ifdef VENUS_INCLUDE_TO_STRING
#include <sstream>
#endif

class VeResult;
namespace venus {
template <typename T> using Result = hermes::Result<T, VeResult>;
} // namespace venus

struct VeResult {
  enum class Type {
    NO_ERROR = 0,         //!< success
    VK_ERROR = 1,         //!< error from a vulkan call
    INCOMPATIBLE_API = 2, //!< api version mismatches and incompatibilities
    NOT_FOUND = 3,        //!< api version mismatches and incompatibilities
    EXT_ERROR = 4,        //!< third party lib error
    CHECK_ERROR = 5,      //!< a check error ocurred
    IO_ERROR = 6,         //!< an io error ocurred
  };

  static VeResult noError() { return {HeError::NO_ERROR, Type::NO_ERROR}; }
  static VeResult incompatible() {
    return {HeError::CUSTOM_ERROR, Type::INCOMPATIBLE_API};
  }
  static VeResult notFound() {
    return {HeError::CUSTOM_ERROR, Type::NOT_FOUND};
  }
  static VeResult vkError() { return {HeError::CUSTOM_ERROR, Type::VK_ERROR}; }
  static VeResult error() { return {HeError::UNKNOWN_ERROR, Type::NO_ERROR}; }
  static VeResult extError() {
    return {HeError::CUSTOM_ERROR, Type::EXT_ERROR};
  }
  static VeResult checkError() {
    return {HeError::CUSTOM_ERROR, Type::CHECK_ERROR};
  }
  static VeResult inputError() {
    return {HeError::INVALID_INPUT, Type::NO_ERROR};
  }
  static VeResult badAllocation() {
    return {HeError::BAD_ALLOCATION, Type::NO_ERROR};
  }
  static VeResult ioError() { return {HeError::CUSTOM_ERROR, Type::IO_ERROR}; }

  VeResult() = default;
  VeResult(Type type) : base_type(HeError::UNKNOWN_ERROR), type(type) {}
  VeResult(HeError base_type, Type type) : base_type(base_type), type(type) {}

  /// \return True if NO_ERROR
  operator bool() const { return base_type == HeError::NO_ERROR; }
  template <typename T> operator venus::Result<T>() const {
    return venus::Result<T>::error(*this);
  }

  HeError base_type{HeError::NO_ERROR};
  Type type{Type::NO_ERROR};
};

namespace venus {

#ifdef VENUS_INCLUDE_TO_STRING
inline std::string to_string(const VeResult &err) {
  std::stringstream ss;
  if (err.base_type != HeError::CUSTOM_ERROR)
    ss << hermes::to_string(err.base_type);
  if (err.base_type != HeError::NO_ERROR) {
#define VE_ERROR_TYPE_NAME(E)                                                  \
  if (err.type == VeResult::Type::E)                                           \
  ss << #E
    VE_ERROR_TYPE_NAME(NO_ERROR);
    VE_ERROR_TYPE_NAME(VK_ERROR);
    VE_ERROR_TYPE_NAME(INCOMPATIBLE_API);
    VE_ERROR_TYPE_NAME(NOT_FOUND);
    VE_ERROR_TYPE_NAME(EXT_ERROR);
    VE_ERROR_TYPE_NAME(CHECK_ERROR);
    VE_ERROR_TYPE_NAME(IO_ERROR);
#undef VE_ERROR_TYPE_NAME
  }
  return ss.str();
}
#endif

} // namespace venus
