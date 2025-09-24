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

/// \file   surface.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-06-07

#include <venus/io/surface.h>

namespace venus::io {

SurfaceKHR::SurfaceKHR(VkInstance vk_instance, VkSurfaceKHR vk_surface) noexcept
    : vk_instance_(vk_instance), vk_surface_(vk_surface) {}

SurfaceKHR::SurfaceKHR(SurfaceKHR &&rhs) noexcept { this->swap(rhs); }

SurfaceKHR::~SurfaceKHR() noexcept { destroy(); }

SurfaceKHR &SurfaceKHR::operator=(SurfaceKHR &&rhs) noexcept {
  destroy();
  this->swap(rhs);
  return *this;
}

void SurfaceKHR::destroy() noexcept {
  if (vk_instance_ && vk_surface_)
    vkDestroySurfaceKHR(vk_instance_, vk_surface_, nullptr);
  vk_instance_ = VK_NULL_HANDLE;
  vk_surface_ = VK_NULL_HANDLE;
}

void SurfaceKHR::swap(SurfaceKHR &rhs) {
  VENUS_SWAP_FIELD_WITH_RHS(vk_surface_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_instance_);
}

VkSurfaceKHR SurfaceKHR::operator*() const { return vk_surface_; }

} // namespace venus::io
