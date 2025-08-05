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

#include <venus/io/glfw_display.h>
#include <venus/utils/vk_debug.h>

namespace venus::io {

GLFW_Context GLFW_Context::instance_;

GLFW_Context::GLFW_Context() noexcept {
  glfwSetErrorCallback([](int error, const char *msg) {
    HERMES_ERROR("glfw: ({}) {}", error, msg);
  });
  glfwInit();
}

GLFW_Context::~GLFW_Context() noexcept { glfwTerminate(); }

GLFW_Window::~GLFW_Window() noexcept { destroy(); }

VeResult GLFW_Window::init(const char *name, const VkExtent2D &extent) {

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  window_ =
      glfwCreateWindow(extent.width, extent.height, name, nullptr, nullptr);
  if (!window_) {
    HERMES_ERROR("Failed to create glfw window.");
    return VeResult::extError();
  }
  resolution_ = extent;

  // setup callbacks

  glfwSetKeyCallback(window_, [](GLFWwindow *window, int key, int scancode,
                                 int action, int mods) {
    HERMES_UNUSED_VARIABLE(window);
    HERMES_UNUSED_VARIABLE(key);
    HERMES_UNUSED_VARIABLE(scancode);
    HERMES_UNUSED_VARIABLE(action);
    HERMES_UNUSED_VARIABLE(mods);
    glfwSetWindowShouldClose(window, true);
  });

  glfwSetCursorPosCallback(window_,
                           [](GLFWwindow *window, double xpos, double ypos) {
                             HERMES_UNUSED_VARIABLE(window);
                             HERMES_UNUSED_VARIABLE(xpos);
                             HERMES_UNUSED_VARIABLE(ypos);
                           });

  glfwSetMouseButtonCallback(
      window_, [](GLFWwindow *window, int button, int action, int mods) {
        HERMES_UNUSED_VARIABLE(window);
        HERMES_UNUSED_VARIABLE(button);
        HERMES_UNUSED_VARIABLE(action);
        HERMES_UNUSED_VARIABLE(mods);
      });

  glfwSetScrollCallback(window_,
                        [](GLFWwindow *window, double xoffset, double yoffset) {
                          HERMES_UNUSED_VARIABLE(window);
                          HERMES_UNUSED_VARIABLE(xoffset);
                          HERMES_UNUSED_VARIABLE(yoffset);
                        });

  return VeResult::noError();
}

VeResult GLFW_Window::destroy() {
  if (window_) {
    glfwDestroyWindow(window_);
    window_ = nullptr;
  }
  return VeResult::noError();
}

Result<VkSurfaceKHR> GLFW_Window::createSurface(VkInstance instance) const {
  VkSurfaceKHR surface;
  VENUS_VK_RETURN_BAD_RESULT(
      glfwCreateWindowSurface(instance, window_, nullptr, &surface));
  return Result<VkSurfaceKHR>(surface);
}

bool GLFW_Window::shouldClose() { return glfwWindowShouldClose(window_); }

void GLFW_Window::pollEvents() { glfwPollEvents(); }

} // namespace venus::io
