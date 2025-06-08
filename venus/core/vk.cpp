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
///\file vk.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2025-06-07
///
///\brief

#include <venus/core/debug.h>
#include <venus/core/vk.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#pragma GCC diagnostic pop

#include <cstdlib>

namespace venus::core {

void sanityCheck() {
  void *ptr;

  // This won't compile if the appropriate Vulkan platform define isn't set.
  ptr =
#if defined(_WIN32)
      &vkCreateWin32SurfaceKHR;
#elif defined(__linux__) || defined(__unix__)
      &vkCreateXlibSurfaceKHR;
#elif defined(__APPLE__)
      &vkCreateMacOSSurfaceMVK;
#else
      NULL;
#endif

  VENUS_UNUSED_VARIABLE(ptr);
}

vk &vk::get() {
  static vk s_vk;
  return s_vk;
}

vk::vk() = default;
vk::~vk() = default;

void vk::init() {
  sanityCheck();

  VkResult r = volkInitialize();
  if (r != VK_SUCCESS) {
    VENUS_ERROR("volkInitialize failed!");
    return;
  }

  auto version = volkGetInstanceVersion();
  VENUS_INFO(std::format("Vulkan version {}.{}.{} initialized.",
                         VK_VERSION_MAJOR(version), VK_VERSION_MINOR(version),
                         VK_VERSION_PATCH(version)));

  auto &v = get();
}

} // namespace venus::core
