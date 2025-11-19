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

/// \file   display_app.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30

#include <venus/app/display_app.h>

#include <venus/utils/macros.h>
#include <venus/utils/vk_debug.h>

namespace venus::app {

template <>
Result<DisplayApp>
DisplayApp::Setup<DisplayApp::Config, DisplayApp>::build() const {

  DisplayApp app;

  app.window_ = display_;
  app.render_callback_ = render_callback_;
  app.startup_callback_ = startup_callback_;
  app.shutdown_callback_ = shutdown_callback_;

  VENUS_RETURN_BAD_RESULT(app.window_->init(title_.c_str(), resolution_));

  app.window_->key_func = key_func_;
  app.window_->mouse_button_func = mouse_button_func_;
  app.window_->cursor_pos_func = cursor_pos_func_;
  app.window_->scroll_func = scroll_func_;

  app.fps_ = fps_;
  app.frames_ = frames_;

  return Result<DisplayApp>(std::move(app));
}

void DisplayApp::swap(DisplayApp &rhs) {
  VENUS_SWAP_FIELD_WITH_RHS(window_);
  VENUS_SWAP_FIELD_WITH_RHS(surface_);
  VENUS_SWAP_FIELD_WITH_RHS(startup_callback_);
  VENUS_SWAP_FIELD_WITH_RHS(shutdown_callback_);
  VENUS_SWAP_FIELD_WITH_RHS(render_callback_);
  VENUS_SWAP_FIELD_WITH_RHS(fps_);
  VENUS_SWAP_FIELD_WITH_RHS(frames_);
}

void DisplayApp::destroy() noexcept {
  window_.reset();
  surface_ = VK_NULL_HANDLE;
  frames_ = 0;
}

i32 DisplayApp::run() {
  if (startup_callback_)
    VENUS_RETURN_ON_BAD_RESULT(startup_callback_(*this), -1);
  for (auto it :
       engine::FrameLoop().setDurationInFrames(frames_).setFPS(fps_)) {
    if (render_callback_)
      VENUS_RETURN_ON_BAD_RESULT(render_callback_(it.frame()), -1);
    if (window_->shouldClose())
      it.endLoop();
    window_->pollEvents();
  }
  if (shutdown_callback_)
    VENUS_RETURN_ON_BAD_RESULT(shutdown_callback_(), -1);
  return shutdown();
}

i32 DisplayApp::shutdown() {
  window_->destroy();
  return 0;
}

const io::Display *DisplayApp::display() const { return window_.get(); }

} // namespace venus::app
