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

#include <venus/engine/frame_loop.h>
#include <venus/io/display.h>

namespace venus::app {

/// Base auxiliary class for providing full desktop graphics application.
class DisplayApp {
public:
  // Configuration

  /// Builder for DisplayApp.
  /// \tparam Derived return type of configuration methods.
  /// \tparam Type type of the object build by this setup.
  template <typename Derived, typename Type> struct Setup {
    ///
    Derived &setTitle(const std::string_view &title);
    ///
    Derived &setResolution(const VkExtent2D &resolution);
    ///
    template <typename DisplayType>
    Derived &setDisplay(const std::string_view &title,
                        const VkExtent2D &resolution);
    ///
    Derived &setFPS(f32 fps);
    /// \param frame_count total number of frames before shutdown.
    /// \note frame_count = 0 means no limit.
    Derived &setDurationInFrames(u32 frame_count);
    ///
    Derived &
    setStartupFn(const std::function<VeResult(Type &)> &startup_callback);
    ///
    Derived &setShutdownFn(const std::function<VeResult()> &shutdown_callback);
    ///
    Derived &setRenderFn(
        const std::function<VeResult(
            const engine::FrameLoop::Iteration::Frame &)> &render_callback);
    ///
    Derived &
    setCursorPosFn(const std::function<void(const hermes::geo::point2 &)>
                       &cursor_pos_func);
    ///
    Derived &setMouseButtonFn(
        const std::function<void(ui::Action, ui::MouseButton, ui::Modifier)>
            &mouse_button_func);
    ///
    Derived &setMouseScrollFn(
        const std::function<void(const hermes::geo::vec2 &)> &scroll_func);
    ///
    Derived &setKeyFn(
        const std::function<void(ui::Action, ui::Key, ui::Modifier)> &key_func);

  protected:
    // display settings
    std::string title_;
    VkExtent2D resolution_{};
    std::shared_ptr<io::Display> display_;
    // callbacks
    std::function<VeResult(Type &)> startup_callback_{nullptr};
    std::function<VeResult()> shutdown_callback_{nullptr};
    std::function<VeResult(const engine::FrameLoop::Iteration::Frame &)>
        render_callback_{nullptr};
    std::function<void(const hermes::geo::point2 &)> cursor_pos_func_;
    std::function<void(ui::Action, ui::MouseButton, ui::Modifier)>
        mouse_button_func_;
    std::function<void(const hermes::geo::vec2 &)> scroll_func_;
    std::function<void(ui::Action, ui::Key, ui::Modifier)> key_func_;
    // display config
    f32 fps_{60.0};
    u32 frames_{0};
  };

  struct Config : public Setup<Config, DisplayApp> {
    ///
    Result<DisplayApp> build() const;
  };

  void swap(DisplayApp &rhs);
  void destroy() noexcept;

  /// Start the application.
  virtual i32 run();

  const io::Display *display() const;

protected:
  i32 shutdown();

  std::shared_ptr<io::Display> window_;
  VkSurfaceKHR surface_{VK_NULL_HANDLE};

  std::function<VeResult(DisplayApp &)> startup_callback_{nullptr};
  std::function<VeResult()> shutdown_callback_{nullptr};
  std::function<VeResult(const engine::FrameLoop::Iteration::Frame &)>
      render_callback_{nullptr};

  f32 fps_{60.0};
  u32 frames_{0};
};

VENUS_DEFINE_SETUP_SET_FIELD_METHOD_T(DisplayApp, setTitle,
                                      const std::string_view &, title_);
VENUS_DEFINE_SETUP_SET_FIELD_METHOD_T(DisplayApp, setResolution,
                                      const VkExtent2D &, resolution_);
VENUS_DEFINE_SETUP_SET_FIELD_METHOD_T(DisplayApp, setStartupFn,
                                      const std::function<VeResult(Type &)> &,
                                      startup_callback_);
VENUS_DEFINE_SETUP_SET_FIELD_METHOD_T(DisplayApp, setShutdownFn,
                                      const std::function<VeResult()> &,
                                      shutdown_callback_);
VENUS_DEFINE_SETUP_SET_FIELD_METHOD_T(
    DisplayApp, setRenderFn,
    const std::function<VeResult(const engine::FrameLoop::Iteration::Frame &)>
        &,
    render_callback_);
VENUS_DEFINE_SETUP_SET_FIELD_METHOD_T(
    DisplayApp, setCursorPosFn,
    const std::function<void(const hermes::geo::point2 &)> &, cursor_pos_func_);
VENUS_DEFINE_SETUP_SET_FIELD_METHOD_T(
    DisplayApp, setMouseButtonFn,
    const std::function<void(ui::Action, ui::MouseButton, ui::Modifier)> &,
    mouse_button_func_);
VENUS_DEFINE_SETUP_SET_FIELD_METHOD_T(
    DisplayApp, setMouseScrollFn,
    const std::function<void(const hermes::geo::vec2 &)> &, scroll_func_);
VENUS_DEFINE_SETUP_SET_FIELD_METHOD_T(
    DisplayApp, setKeyFn,
    const std::function<void(ui::Action, ui::Key, ui::Modifier)> &, key_func_);
VENUS_DEFINE_SETUP_SET_FIELD_METHOD_T(DisplayApp, setFPS, f32, fps_);
VENUS_DEFINE_SETUP_SET_FIELD_METHOD_T(DisplayApp, setDurationInFrames, u32,
                                      frames_);

template <typename Derived, typename Type>
template <typename DisplayType>
Derived &
DisplayApp::Setup<Derived, Type>::setDisplay(const std::string_view &title,
                                             const VkExtent2D &resolution) {
  display_.reset(new DisplayType());
  title_ = title;
  resolution_ = resolution;
  return static_cast<Derived &>(*this);
}

} // namespace venus::app
