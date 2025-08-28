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

#include <venus/utils/vk_debug.h>

std::vector<std::string> getInstanceExtensions() {
  std::vector<std::string> extensions;
  extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
  extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_METAL_EXT)
  extensions.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_VI_NN)
  extensions.push_back(VK_NN_VI_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
  extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
  extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
  extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
  extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
  extensions.push_back(VK_EXT_ACQUIRE_XLIB_DISPLAY_EXTENSION_NAME);
#endif
  return extensions;
}

namespace venus::app {

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(DisplayApp, setTitle,
                                     const std::string_view &, title_ = value);
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(DisplayApp, setResolution,
                                     const VkExtent2D &, resolution_ = value);
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(DisplayApp, setDeviceFeatures,
                                     const core::vk::DeviceFeatures &,
                                     device_features_ = value);
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(DisplayApp, setDeviceExtensions,
                                     const std::vector<std::string> &,
                                     device_extensions_ = value);
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(DisplayApp, setStartupFn,
                                     const std::function<VeResult()> &,
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

  VENUS_ASSIGN_RESULT(app.instance_,
                      core::Instance::Config()
                          .setApiVersion(core::vk::Version(1, 4, 0))
                          .setName("hello_vulkan_app")
                          .addExtensions(getInstanceExtensions())
                          .enableDefaultDebugMessageSeverityFlags()
                          .enableDefaultDebugMessageTypeFlags()
                          .enableDebugUtilsExtension()
                          .create());
  HERMES_INFO("\n{}", venus::to_string(app.instance_));

  app.window_ = display_;

  VENUS_CHECK_VE_RESULT(app.window_->init(title_.c_str(), resolution_));

  VENUS_ASSIGN_RESULT(app.surface_, app.window_->createSurface(*app.instance_));

  VENUS_ASSIGN_RESULT(app.gd_, GraphicsDevice::Config()
                                   .setSurface(app.surface_)
                                   .setSurfaceExtent(app.window_->resolution())
                                   .addExtensions(device_extensions_)
                                   .create(app.instance_));
  return app;
}

i32 DisplayApp::run() {
  if (startup_callback_)
    startup_callback_();
  for (auto it : io::DisplayLoop(*window_).setDurationInFrames(10)) {
    HERMES_INFO("{}", venus::to_string(it));
    if (render_callback_)
      render_callback_(it.frame());
  }
  if (shutdown_callback_)
    shutdown_callback_();
  return shutdown();
}

i32 DisplayApp::shutdown() {
  gd_.destroy();
  window_->destroy();
  instance_.destroy();
  return 0;
}

} // namespace venus::app
