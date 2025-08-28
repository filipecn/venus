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

/// \file   display.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-06-07

#include <venus/io/display.h>

namespace venus::io {

DisplayLoop::Iteration::Iteration(DisplayLoop &loop, bool is_end)
    : loop_{loop}, is_end_{is_end} {}

DisplayLoop::Iteration &DisplayLoop::Iteration::operator++() {
  if (in_frame_) {
    // finish frame
    in_frame_ = false;
    auto frame_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - frame_.frame_start);
    // ensure fps
    if (frame_duration < loop_.fps_period_)
      std::this_thread::sleep_until(frame_.frame_start + loop_.fps_period_);
    frame_.last_frame_duration = frame_duration;
    frame_.current_fps_period =
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - frame_.frame_start);
  }
  frame_.iteration_index++;
  if (loop_.max_frame_count_ &&
      frame_.iteration_index >= loop_.max_frame_count_) {
    is_end_ = true;
    return *this;
  }
  if (loop_.display_.shouldClose()) {
    is_end_ = true;
    return *this;
  }
  in_frame_ = true;
  frame_.frame_start = std::chrono::steady_clock::now();
  loop_.display_.pollEvents();
  return *this;
}

DisplayLoop::Iteration &DisplayLoop::Iteration::operator*() { return *this; }

bool DisplayLoop::Iteration::operator==(
    const DisplayLoop::Iteration &rhs) const {
  return is_end_ == rhs.is_end_;
}

bool DisplayLoop::Iteration::operator!=(
    const DisplayLoop::Iteration &rhs) const {
  return is_end_ != rhs.is_end_;
}

const DisplayLoop::Iteration::Frame &DisplayLoop::Iteration::frame() const {
  return frame_;
}

DisplayLoop::DisplayLoop(Display &display) : display_(display) {}

DisplayLoop &DisplayLoop::setFPS(f32 fps) {
  fps_period_ = std::chrono::microseconds(u64(1000000 / fps));
  return *this;
}

DisplayLoop &DisplayLoop::setDurationInFrames(u32 frame_count) {
  max_frame_count_ = frame_count;
  return *this;
}

DisplayLoop::Iteration DisplayLoop::begin() { return {*this, false}; }

DisplayLoop::Iteration DisplayLoop::end() { return {*this, true}; }

} // namespace venus::io

namespace venus {

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::io::DisplayLoop::Iteration)
HERMES_PUSH_DEBUG_FIELD(frame_.iteration_index);
HERMES_PUSH_DEBUG_FIELD(frame_.last_frame_duration.count());
HERMES_PUSH_DEBUG_FIELD(frame_.current_fps_period.count());
HERMES_TO_STRING_DEBUG_METHOD_END

} // namespace venus
