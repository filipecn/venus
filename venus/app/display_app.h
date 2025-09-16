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

/// \file   display_app.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-06-07
/// \brief  Graphics application.

#pragma once

#include <venus/io/display.h>

namespace venus::app {

/// Base auxiliary class for providing full desktop graphics application.
class DisplayApp {
public:
  // Configuration

  struct Config {
    Config &setTitle(const std::string_view &title);
    Config &setResolution(const VkExtent2D &resolution);
    template <typename DisplayType>
    Config &setDisplay(const std::string_view &title,
                       const VkExtent2D &resolution) {
      display_.reset(new DisplayType());
      title_ = title;
      resolution_ = resolution;
      return *this;
    }
    Config &
    setStartupFn(const std::function<VeResult(DisplayApp &)> &startup_callback);
    Config &setShutdownFn(const std::function<VeResult()> &shutdown_callback);
    Config &setRenderFn(
        const std::function<VeResult(const io::DisplayLoop::Iteration::Frame &)>
            &render_callback);

    DisplayApp create() const;

  private:
    // display settings
    std::string title_;
    VkExtent2D resolution_{};
    std::shared_ptr<io::Display> display_;
    // callbacks
    std::function<VeResult(DisplayApp &)> startup_callback_{nullptr};
    std::function<VeResult()> shutdown_callback_{nullptr};
    std::function<VeResult(const io::DisplayLoop::Iteration::Frame &)>
        render_callback_{nullptr};
  };

  /// Start the application.
  i32 run();

  const io::Display *display() const;

protected:
  i32 shutdown();

  std::shared_ptr<io::Display> window_;
  VkSurfaceKHR surface_{VK_NULL_HANDLE};

  std::function<VeResult(DisplayApp &)> startup_callback_{nullptr};
  std::function<VeResult()> shutdown_callback_{nullptr};
  std::function<VeResult(const io::DisplayLoop::Iteration::Frame &)>
      render_callback_{nullptr};
};

} // namespace venus::app
