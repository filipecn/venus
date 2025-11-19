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

/// \file   frame_loop.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-11-15
/// \brief  Frame loop.

#pragma once

#include <venus/utils/debug.h>

namespace venus::engine {

/// Auxiliary class for looping over frame indices keeping a FPS
///
/// This class is meant to be used in a for loop as:
/// for (auto it : FrameLoop()) {
///   // iteration info can be accessed via 'it':
///   it.frame();
/// }
/// The frame loop can be configured as well:
/// FrameLoop().setFPS(...).setDurationInFrames(...)
///
/// \note The FPS is ensured by calling the std::this_thread::sleep method
class FrameLoop {
public:
  FrameLoop &setFPS(f32 fps);
  FrameLoop &setDurationInFrames(u32 frame_count);

  struct Iteration {
    struct Frame {
      u32 iteration_index{0};
      // fps
      std::chrono::steady_clock::time_point frame_start;
      std::chrono::microseconds last_frame_duration{0};
      std::chrono::microseconds current_fps_period{0};
    };

    Iteration(FrameLoop &loop, bool is_end);

    Iteration &operator++();
    Iteration &operator*();
    bool operator==(const Iteration &) const;
    bool operator!=(const Iteration &) const;

    const Frame &frame() const;
    void endLoop();

  private:
    FrameLoop &loop_;
    bool in_frame_{false};
    bool is_end_{false};
    Frame frame_;

    VENUS_to_string_FRIEND(Iteration);
  };

  Iteration begin();
  Iteration end();

private:
  friend class Iteration;

  // default for 60 fps
  std::chrono::microseconds fps_period_{16666};
  u32 max_frame_count_{0};
};

} // namespace venus::engine
