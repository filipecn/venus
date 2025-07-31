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

/// \file   glfw_display.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-06-07
/// \brief  GLFW Display

#pragma once

#include "hermes/core/types.h"
#include <venus/io/display.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace venus::io {

class GLFW_Context {
public:
  ~GLFW_Context() noexcept;
  GLFW_Context(const GLFW_Context &other) = delete;
  GLFW_Context &operator=(const GLFW_Context &other) = delete;

  static VkResult enter();
  static void leave();

private:
  GLFW_Context() noexcept;

  static GLFW_Context instance_;

  bool initialized_{false};
  u32 client_count_{0};
};

class GLFW_Window : public Display {
public:
  GLFW_Window() noexcept = default;
  ~GLFW_Window() noexcept override;

  HERMES_NODISCARD VeResult init(const char *name, const VkExtent2D &extent);

  // interface

  /// Creates a vulkan surface object from a given instance handle.
  /// \note The caller is responsible for destroying the newly created object.
  Result<VkSurfaceKHR> createSurface(VkInstance vk_instance) const override;
  /// Destroy this display resources.
  VeResult destroy() override;
  /// \return true if the display is closing and must be destroyed.
  bool shouldClose() override;
  /// Receive new input events.
  void pollEvents() override;

private:
  GLFWwindow *window_{nullptr};
};

} // namespace venus::io
