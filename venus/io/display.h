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

/// \file   display.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-06-07
/// \brief  Display interface.

#pragma once

#include <hermes/core/types.h>
#include <venus/core/vk_api.h>
#include <venus/io/surface.h>

namespace venus::io {

/// \brief Interface for display objects.
/// Displays represent the main interaction with users by presenting the output
/// images and handling input events.
class Display {
public:
  Display() = default;
  virtual ~Display() = default;

  // interface

  virtual VeResult init(const char *title, const VkExtent2D &size) = 0;
  /// Creates a vulkan surface object from a given instance handle.
  /// \note The caller is responsible for destroying the newly created object.
  ///       Unless the child class does it.
  virtual Result<SurfaceKHR> createSurface(VkInstance vk_instance) const = 0;
  /// Destroy this display resources.
  virtual VeResult destroy() = 0;
  /// \return true if the display is closing and must be destroyed.
  virtual bool shouldClose() = 0;
  /// Receive new input events.
  virtual void pollEvents() = 0;

  // base methods

  /// \return The display size in pixels.
  const VkExtent2D &resolution() const { return resolution_; }
  /// \return Screen ratio (width / height)
  f32 ratio() const {
    HERMES_ASSERT(resolution_.height);
    return resolution_.width / static_cast<f32>(resolution_.height);
  }

protected:
  VkExtent2D resolution_{};
};

class DisplayLoop {
public:
  DisplayLoop(Display &display);
  DisplayLoop &setFPS(f32 fps);
  DisplayLoop &setDurationInFrames(u32 frame_count);

  struct Iteration {
    struct Frame {
      u32 iteration_index{0};
      // fps
      std::chrono::steady_clock::time_point frame_start;
      std::chrono::microseconds last_frame_duration{0};
      std::chrono::microseconds current_fps_period{0};
    };

    Iteration(DisplayLoop &loop, bool is_end);

    Iteration &operator++();
    Iteration &operator*();
    bool operator==(const Iteration &) const;
    bool operator!=(const Iteration &) const;
    const Frame &frame() const;

  private:
    DisplayLoop &loop_;
    bool in_frame_{false};
    bool is_end_{false};
    Frame frame_;

    VENUS_TO_STRING_FRIEND(Iteration);
  };

  Iteration begin();
  Iteration end();

private:
  friend class Iteration;

  Display &display_;
  // default for 60 fps
  std::chrono::microseconds fps_period_{16666};
  u32 max_frame_count_{0};
};

} // namespace venus::io
