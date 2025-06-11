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
///\file surface.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2025-06-07
///
///\brief

#pragma once

#include <venus/core/instance.h>

namespace venus::core {

/// Holds a surface handle to native platform surface or window objects.
/// Separate platform-specific extensions each provide a function for creating
/// a VkSurfaceKHR object for the respective platform.
/// \note This class uses RAII
class SurfaceKHR {
public:
  // ***************************************************************************
  //                                                              CONSTRUCTORS
  // ***************************************************************************
  SurfaceKHR();
  SurfaceKHR(const Instance &instance, VkSurfaceKHR vk_surface_handle);
  SurfaceKHR(const SurfaceKHR &other) = delete;
  SurfaceKHR(SurfaceKHR &&other) noexcept;
  virtual ~SurfaceKHR();
  // ***************************************************************************
  //                                                                 OPERATORS
  // ***************************************************************************
  SurfaceKHR &operator=(const SurfaceKHR &other) = delete;
  SurfaceKHR &operator=(SurfaceKHR &&other) noexcept;
  // ***************************************************************************
  //                                                                   METHODS
  // ***************************************************************************
  void destroy();
  [[nodiscard]] VkSurfaceKHR handle() const;
  [[nodiscard]] bool good() const;

private:
  VkInstance vk_instance_{VK_NULL_HANDLE};
  VkSurfaceKHR vk_surface_handle_{VK_NULL_HANDLE};
};

} // namespace venus::core
