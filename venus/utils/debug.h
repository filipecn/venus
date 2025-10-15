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

/// \file   debug.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-06-07
/// \brief  Auxiliary functions for debugging and error handling.

#pragma once

#include <cassert>
#include <venus/utils/result.h>

#include <hermes/core/debug.h>

#ifdef VENUS_INCLUDE_TO_STRING

namespace venus {
HERMES_TEMPLATE_TO_STRING_DEBUG_METHOD;
} // namespace venus
#define VENUS_to_string_FRIEND(A)                                              \
  friend std::string venus::to_string(const A &, u32)
#define VENUS_VIRTUAL_toString_METHOD                                          \
  virtual std::string toString(u32 tab_size = 0) const;
#define VENUS_VIRTUAL_toString_METHOD_OVERRIDE                                 \
  virtual std::string toString(u32 tab_size = 0) const override;

#ifndef HERMES_PUSH_DEBUG_VENUS_FIELD
#define HERMES_PUSH_DEBUG_VENUS_FIELD(F)                                       \
  debug_fields.add(HERMES_DebugFields::Type::Inline, #F,                       \
                   venus::to_string(object.F));
#endif

#ifndef HERMES_PUSH_DEBUG_VENUS_PTR_FIELD
#define HERMES_PUSH_DEBUG_VENUS_PTR_FIELD(F)                                   \
  debug_fields.add(HERMES_DebugFields::Type::Inline, #F,                       \
                   object.F ? venus::to_string(*object.F) : "NULL");
#endif

#else
#define VENUS_to_string_FRIEND
#define HERMES_PUSH_DEBUG_VENUS_FIELD
#endif

#ifndef VENUS_CHECK_VE_RESULT
#define VENUS_CHECK_VE_RESULT(A)                                               \
  {                                                                            \
    VeResult _venus_check_ve_error_ = (A);                                     \
    if (!_venus_check_ve_error_) {                                             \
      HERMES_ERROR("Error at: {}", #A);                                        \
      HERMES_ERROR("  w/ err: {}", venus::to_string(_venus_check_ve_error_));  \
    }                                                                          \
  }
#endif
#ifndef VENUS_CHECK_OR_RESULT
#define VENUS_CHECK_OR_RESULT(A)                                               \
  {                                                                            \
    if (!(A)) {                                                                \
      HERMES_ERROR("Check error: {}", #A);                                     \
      return VeResult::checkError();                                           \
    }                                                                          \
  }
#endif
#ifndef VENUS_RETURN_BAD_RESULT
#define VENUS_RETURN_BAD_RESULT(A)                                             \
  {                                                                            \
    VeResult _venus_return_ve_error_ = (A);                                    \
    if (!_venus_return_ve_error_) {                                            \
      HERMES_ERROR("Error at: {}", #A);                                        \
      HERMES_ERROR("  w/ err: {}", venus::to_string(_venus_return_ve_error_)); \
      return _venus_return_ve_error_;                                          \
    }                                                                          \
  }
#endif

#ifndef VENUS_ASSIGN_RESULT
#define VENUS_ASSIGN_RESULT(R, V)                                              \
  if (auto _venus_result_ = V)                                                 \
    R = std::move(*_venus_result_);                                            \
  else {                                                                       \
    HERMES_ERROR("Error at: {} = {}", #R, #V);                                 \
    HERMES_ERROR("  w/ err: {}", venus::to_string(_venus_result_.status()));   \
  }
#endif

#ifndef VENUS_ASSIGN_RESULT_OR
#define VENUS_ASSIGN_RESULT_OR(R, V, O)                                        \
  if (auto _venus_result_ = V)                                                 \
    R = std::move(*_venus_result_);                                            \
  else {                                                                       \
    HERMES_ERROR("Error at: {} = {}", #R, #V);                                 \
    HERMES_ERROR("  w/ err: {}", venus::to_string(_venus_result_.status()));   \
    O;                                                                         \
  }
#endif

#ifndef VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT
#define VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(R, V)                         \
  if (auto _venus_result_ = V)                                                 \
    R = std::move(*_venus_result_);                                            \
  else {                                                                       \
    HERMES_ERROR("Error at: {} = {}", #R, #V);                                 \
    HERMES_ERROR("  w/ err: {}", venus::to_string(_venus_result_.status()));   \
    return _venus_result_.status();                                            \
  }

#endif

#ifndef VENUS_DECLARE_OR_RETURN_BAD_RESULT
#define VENUS_DECLARE_OR_RETURN_BAD_RESULT(TYPE, VAR, V)                       \
  auto _venus_result_##VAR = V;                                                \
  if (!_venus_result_##VAR) {                                                  \
    HERMES_ERROR("Error at: {}::Ptr {} = {}", #TYPE, #VAR, #V);                \
    HERMES_ERROR("  w/ err: {}",                                               \
                 venus::to_string(_venus_result_##VAR.status()));              \
    return _venus_result_##VAR.status();                                       \
  }                                                                            \
  TYPE VAR = std::move(*_venus_result_##VAR);

#endif

#ifndef VENUS_DECLARE_SHARED_PTR_FROM_RESULT_OR_RETURN_BAD_RESULT
#define VENUS_DECLARE_SHARED_PTR_FROM_RESULT_OR_RETURN_BAD_RESULT(TYPE, PTR,   \
                                                                  V)           \
  auto PTR = TYPE::Ptr::shared();                                              \
  if (auto _venus_result_ = V) {                                               \
    *PTR = std::move(*_venus_result_);                                         \
  } else {                                                                     \
    HERMES_ERROR("Error at: {}::Ptr {} = {}", #TYPE, #PTR, #V);                \
    HERMES_ERROR("  w/ err: {}", venus::to_string(_venus_result_.status()));   \
    return _venus_result_.status();                                            \
  }

#endif

#ifndef VENUS_ASSIGN_RESULT_OR_RETURN
#define VENUS_ASSIGN_RESULT_OR_RETURN(R, V, B)                                 \
  if (auto _venus_result_ = V)                                                 \
    R = std::move(*_venus_result_);                                            \
  else {                                                                       \
    HERMES_ERROR("Error at: {} = {}", #R, #V);                                 \
    HERMES_ERROR("  w/ err: {}", venus::to_string(_venus_result_.status()));   \
    return B;                                                                  \
  }
#endif

#ifndef VENUS_ASSIGN_RESULT_OR_RETURN_VOID
#define VENUS_ASSIGN_RESULT_OR_RETURN_VOID(R, V)                               \
  if (auto _venus_result_ = V)                                                 \
    R = std::move(*_venus_result_);                                            \
  else {                                                                       \
    HERMES_ERROR("Error at: {} = {}", #R, #V);                                 \
    HERMES_ERROR("  w/ err: {}", venus::to_string(_venus_result_.status()));   \
    return;                                                                    \
  }
#endif
