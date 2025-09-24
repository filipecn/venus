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

/// \file   surface.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-06-07
/// \brief  Surface KHR

#pragma once

#include <venus/core/vk_api.h>
#include <venus/utils/macros.h>

namespace venus::io {

class SurfaceKHR {
public:
  SurfaceKHR(VkInstance vk_instance, VkSurfaceKHR vk_surface) noexcept;

  VENUS_DECLARE_RAII_FUNCTIONS(SurfaceKHR)

  void destroy() noexcept;
  void swap(SurfaceKHR &rhs);

  VkSurfaceKHR operator*() const;

private:
  VkSurfaceKHR vk_surface_{VK_NULL_HANDLE};
  VkInstance vk_instance_{VK_NULL_HANDLE};
};

} // namespace venus::io
