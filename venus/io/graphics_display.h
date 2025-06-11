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
/// \file graphics_display.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date 2025-06-07
///
/// \brief

#pragma once

#include <GLFW/glfw3.h>
#include <functional>
#include <hermes/common/size.h>
#include <venus/core/vk.h>
#include <venus/io/surface_khr.h>

namespace venus::core {

/// Represents a window used by the application to display graphics and interact
/// with the user.
/// \note This class uses RAII
class GLFW_Display {
public:
  // ***********************************************************************
  //                           CONSTRUCTORS
  // ***********************************************************************
  GLFW_Display();
  /// \brief Construct a new Graphics Display object
  /// \param resolution **[in]** window resolution (in pixels)
  /// \param title **[in]** window title text
  explicit GLFW_Display(
      const hermes::size2 &resolution,
      const std::string &title = std::string("Vulkan Display Window"));
  /// \brief Destroy the Graphics Display object
  ~GLFW_Display();
  // ***********************************************************************
  //                           CREATION
  // ***********************************************************************
  bool init(const hermes::size2 &resolution,
            const std::string &title = std::string("Vulkan Display Window"));
  // ***********************************************************************
  //                           METHODS
  // ***********************************************************************
  /// \return window resolution
  [[nodiscard]] const hermes::size2 &size() const;
  ///
  /// \return
  [[nodiscard]] VkExtent2D framebufferSize() const;
  /// Runs window loop
  void open(const std::function<void()> & = []() {});
  /// Stops window loop
  void close();
  /// \return bool true if window is running
  [[nodiscard]] bool isOpen() const;
  /// \return std::vector<const char *> list of vulkan extensions required by
  /// the GLFW_Display. The hardware must support these extensions in order
  /// to allow the application to use the GLFW_Display.
  [[nodiscard]] static std::vector<const char *> requiredVkExtensions();
  /// \brief Create a Window Surface object
  /// \param instance **[in]** vulkan instance handle
  /// \return SurfaceKHR object (call good() to check if creation was
  /// successful)
  SurfaceKHR createWindowSurface(const Instance &instance);
  ///
  [[maybe_unused]] void waitForValidWindowSize();
  ///
  /// \return
  GLFWwindow *handle();
  // ***********************************************************************
  //                           CALLBACKS
  // ***********************************************************************
  ///
  std::function<void(int, int)> resize_callback;

private:
  hermes::size2 resolution_;
  GLFWwindow *window_ = nullptr;
};

} // namespace venus::core
