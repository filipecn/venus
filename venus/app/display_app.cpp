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

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(DisplayApp, setTitle,
                                     const std::string_view &, title_ = value);
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(DisplayApp, setResolution,
                                     const VkExtent2D &, resolution_ = value);
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(
    DisplayApp, setStartupFn, const std::function<VeResult(DisplayApp &)> &,
    startup_callback_ = value);
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(DisplayApp, setShutdownFn,
                                     const std::function<VeResult()> &,
                                     shutdown_callback_ = value);
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(
    DisplayApp, setRenderFn,
    const std::function<VeResult(const io::DisplayLoop::Iteration::Frame &)> &,
    render_callback_ = value);

DisplayApp DisplayApp::Config::create() const {

  DisplayApp app;

  app.window_ = display_;
  app.render_callback_ = render_callback_;
  app.startup_callback_ = startup_callback_;
  app.shutdown_callback_ = shutdown_callback_;

  VENUS_CHECK_VE_RESULT(app.window_->init(title_.c_str(), resolution_));

  return app;
}

i32 DisplayApp::run() {
  if (startup_callback_)
    startup_callback_(*this);
  for (auto it : io::DisplayLoop(*window_).setDurationInFrames(1)) {
    HERMES_INFO("{}", venus::to_string(it));
    if (render_callback_)
      render_callback_(it.frame());
  }
  if (shutdown_callback_)
    shutdown_callback_();
  return shutdown();
}

i32 DisplayApp::shutdown() {
  window_->destroy();
  return 0;
}

const io::Display *DisplayApp::display() const { return window_.get(); }

} // namespace venus::app
