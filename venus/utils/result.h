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

#pragma once

#define VENUS_DEBUG

#include <sstream>
#include <string>

struct VeResult;
namespace venus {
template <typename T> using Result = hermes::Result<T, VeResult>;
} // namespace venus

struct VeResult {
  enum class Type {
    NoError = 0,         //!< success
    VkError = 1,         //!< error from a vulkan call
    IncompatibleApi = 2, //!< api version mismatches and incompatibilities
    NotFound = 3,        //!< api version mismatches and incompatibilities
    ExtError = 4,        //!< third party lib error
    CheckError = 5,      //!< a check error ocurred
    IOError = 6,         //!< an io error ocurred
  };

  static VeResult noError() { return {HeError::None, Type::NoError}; }
  static VeResult incompatible() {
    return {HeError::Custom, Type::IncompatibleApi};
  }
  static VeResult notFound() { return {HeError::Custom, Type::NotFound}; }
  static VeResult outOfBounds() {
    return {HeError::OutOfBounds, Type::NoError};
  }
  static VeResult vkError() { return {HeError::Custom, Type::VkError}; }
  static VeResult error() { return {HeError::Unknown, Type::NoError}; }
  static VeResult extError() { return {HeError::Custom, Type::ExtError}; }
  static VeResult checkError() { return {HeError::Custom, Type::CheckError}; }
  static VeResult inputError() {
    return {HeError::InvalidInput, Type::NoError};
  }
  static VeResult badAllocation() {
    return {HeError::BadAllocation, Type::NoError};
  }
  static VeResult ioError() { return {HeError::Custom, Type::IOError}; }
  static VeResult heError(HeError he) { return {he, Type::NoError}; }

  VeResult() = default;
  VeResult(Type type) : base_type(HeError::Unknown), type(type) {}
  VeResult(HeError base_type, Type type) : base_type(base_type), type(type) {}

  /// \return True if HeError::None
  operator bool() const { return base_type == HeError::None; }
  template <typename T> operator venus::Result<T>() const {
    return venus::Result<T>::error(*this);
  }

  HeError base_type{HeError::None};
  Type type{Type::NoError};
};

namespace hermes {

inline std::string to_string(const VeResult &err) {
  std::stringstream ss;
  if (err.base_type != HeError::Custom)
    ss << hermes::to_string(err.base_type);
  if (err.base_type != HeError::None) {
#define VE_ERROR_TYPE_NAME(E)                                                  \
  if (err.type == VeResult::Type::E)                                           \
  ss << " | " << #E
    VE_ERROR_TYPE_NAME(NoError);
    VE_ERROR_TYPE_NAME(VkError);
    VE_ERROR_TYPE_NAME(IncompatibleApi);
    VE_ERROR_TYPE_NAME(NotFound);
    VE_ERROR_TYPE_NAME(ExtError);
    VE_ERROR_TYPE_NAME(CheckError);
    VE_ERROR_TYPE_NAME(IOError);
#undef VE_ERROR_TYPE_NAME
  }
  return ss.str();
}

} // namespace hermes
