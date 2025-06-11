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
///\file surface_khr.cpp.c
///\author FilipeCN (filipedecn@gmail.com)
///\date 2025-06-07
///
///\brief

#include <venus/io/surface_khr.h>

namespace venus::core {

SurfaceKHR::SurfaceKHR() = default;

SurfaceKHR::SurfaceKHR(const Instance &instance, VkSurfaceKHR vk_surface_handle)
    : vk_instance_{instance.handle()}, vk_surface_handle_{vk_surface_handle} {}

SurfaceKHR::SurfaceKHR(SurfaceKHR &&other) noexcept {
  destroy();
  vk_instance_ = other.vk_instance_;
  vk_surface_handle_ = other.vk_surface_handle_;
  other.vk_instance_ = VK_NULL_HANDLE;
  other.vk_surface_handle_ = VK_NULL_HANDLE;
}

SurfaceKHR::~SurfaceKHR() { destroy(); }

SurfaceKHR &SurfaceKHR::operator=(SurfaceKHR &&other) noexcept {
  vk_instance_ = other.vk_instance_;
  vk_surface_handle_ = other.vk_surface_handle_;
  other.vk_instance_ = VK_NULL_HANDLE;
  other.vk_surface_handle_ = VK_NULL_HANDLE;
  return *this;
}

void SurfaceKHR::destroy() {
  if (vk_instance_ && vk_surface_handle_)
    vkDestroySurfaceKHR(vk_instance_, vk_surface_handle_, nullptr);
  vk_surface_handle_ = VK_NULL_HANDLE;
}

VkSurfaceKHR SurfaceKHR::handle() const { return vk_surface_handle_; }

bool SurfaceKHR::good() const {
  return vk_instance_ != VK_NULL_HANDLE && vk_surface_handle_ != VK_NULL_HANDLE;
}

} // namespace venus::core
