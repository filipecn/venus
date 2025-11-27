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

#include <venus/app/display_app.h>
#include <venus/app/scene.h>
#include <venus/pipeline/rasterizer.h>
#include <venus/pipeline/ray_tracer.h>
#include <venus/ui/camera.h>

namespace venus::app {

/// Auxiliary class for providing an application with display, scene and camera.
class SceneApp : public DisplayApp {
public:
  /// Builder for SceneApp.
  /// \tparam Derived return type of configuration methods.
  /// \tparam Type type of the object build by this setup.
  template <typename Derived, typename Type>
  struct Setup : DisplayApp::Setup<Derived, Type> {
    /// \param startup_callback Function called during frame preparation.
    Derived &setUpdateSceneFn(
        const std::function<VeResult(Scene &)> &update_scene_callback);

  protected:
    std::function<VeResult(Scene &)> update_scene_callback_{nullptr};
  };

  struct Config : public Setup<Config, SceneApp> {};

  VENUS_DECLARE_RAII_FUNCTIONS(SceneApp)
  virtual void destroy() noexcept;
  void swap(SceneApp &rhs);

  Scene &scene();
  void selectCamera(const std::string &label);

  i32 run() override;

protected:
  VeResult setupCallbacks();
  virtual VeResult init() = 0;
  virtual VeResult render(const engine::FrameLoop::Iteration::Frame &frame) = 0;
  virtual VeResult shutdown() = 0;

  Scene scene_;
  std::string selected_camera_;
  ui::CameraController camera_controller_;

  // scene app callbacks
  std::function<VeResult(Scene &)> update_scene_callback_{nullptr};
  // display app callbacks
  std::function<VeResult()> sa_shutdown_callback_{nullptr};
  std::function<VeResult(const engine::FrameLoop::Iteration::Frame &)>
      sa_render_callback_{nullptr};
};

VENUS_DEFINE_SETUP_SET_FIELD_METHOD_T(SceneApp, setUpdateSceneFn,
                                      const std::function<VeResult(Scene &)> &,
                                      update_scene_callback_)

class RA_SceneApp : public SceneApp {
public:
  struct Config : public SceneApp::Setup<Config, RA_SceneApp> {
    Result<RA_SceneApp> build() const;
  };

  VENUS_DECLARE_RAII_FUNCTIONS(RA_SceneApp)
  void destroy() noexcept override;
  void swap(RA_SceneApp &rhs);

protected:
  VeResult init() override;
  VeResult render(const engine::FrameLoop::Iteration::Frame &frame) override;
  VeResult shutdown() override;

private:
  std::function<VeResult(RA_SceneApp &)> sa_startup_callback_{nullptr};

  /// Global descriptor allocator
  pipeline::DescriptorAllocator descriptor_allocator_;
  /// The global descriptor set is bound at the beginning of the array of
  /// descriptor sets accessed by all render objects.
  pipeline::DescriptorSet global_descriptor_set_;
};

class RT_SceneApp : public SceneApp {
public:
  struct Config : public SceneApp::Setup<Config, RT_SceneApp> {
    Result<RT_SceneApp> build() const;
  };

  VENUS_DECLARE_RAII_FUNCTIONS(RT_SceneApp)
  void destroy() noexcept override;
  void swap(RT_SceneApp &rhs);

protected:
  VeResult init() override;
  VeResult render(const engine::FrameLoop::Iteration::Frame &frame) override;
  VeResult shutdown() override;

private:
  std::function<VeResult(RT_SceneApp &)> sa_startup_callback_{nullptr};
  pipeline::RayTracer ray_tracer_;
};

} // namespace venus::app
