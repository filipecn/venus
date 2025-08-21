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

/// \file   macros.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30
/// \brief  Utility macros.

#pragma once

#ifndef VENUS_DECLARE_RAII_FUNCTIONS
#define VENUS_DECLARE_RAII_FUNCTIONS(CLASS)                                    \
  CLASS() = default;                                                           \
  virtual ~CLASS() noexcept;                                                   \
  CLASS(const CLASS &) = delete;                                               \
  CLASS(CLASS &&) noexcept;                                                    \
  CLASS &operator=(const CLASS &) = delete;                                    \
  CLASS &operator=(CLASS &&) noexcept;
#endif

#ifndef VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD
#define VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(CLASS, METHOD, TYPE, FIELD)  \
  CLASS &CLASS::METHOD(TYPE value) {                                           \
    info_.FIELD = value;                                                       \
    return *this;                                                              \
  }
#endif

#ifndef VENUS_DEFINE_SET_CONFIG_FIELD_METHOD
#define VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(CLASS, METHOD, TYPE, ASSIGNMENT)  \
  CLASS &CLASS::METHOD(TYPE value) {                                           \
    (ASSIGNMENT);                                                              \
    return *this;                                                              \
  }
#endif

#ifndef VENUS_SWAP_FIELD_WITH_RHS
#define VENUS_SWAP_FIELD_WITH_RHS(FIELD) std::swap(FIELD, rhs.FIELD)
#endif

#ifndef VENUS_FIELD_SWAP_RHS
#define VENUS_FIELD_SWAP_RHS(FIELD) FIELD.swap(rhs.FIELD)
#endif
