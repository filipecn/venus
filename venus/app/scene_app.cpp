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

#include <venus/app/scene_app.h>

#include <venus/utils/macros.h>
#include <venus/utils/vk_debug.h>

#include <imgui.h>

namespace venus::app {

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(
    SceneApp, setStartupFn, const std::function<VeResult(SceneApp &)> &,
    startup_callback_ = value);

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(SceneApp, setUpdateSceneFn,
                                     const std::function<VeResult(Scene &)> &,
                                     update_scene_callback_ = value);

SceneApp SceneApp::Config::create() const {
  SceneApp app;
  app.init(display_app_config_);
  app.startup_callback_ = startup_callback_;
  app.update_scene_callback_ = update_scene_callback_;
  return app;
}

SceneApp::~SceneApp() noexcept { destroy(); }

SceneApp::SceneApp(SceneApp &&rhs) noexcept { *this = std::move(rhs); }

SceneApp &SceneApp::operator=(SceneApp &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void SceneApp::destroy() noexcept {
  renderer_.destroy();
  scene_.destroy();
}

void SceneApp::swap(SceneApp &rhs) {
  VENUS_SWAP_FIELD_WITH_RHS(display_app_);
  VENUS_SWAP_FIELD_WITH_RHS(renderer_);
  VENUS_SWAP_FIELD_WITH_RHS(scene_);
  VENUS_SWAP_FIELD_WITH_RHS(da_startup_callback_);
  VENUS_SWAP_FIELD_WITH_RHS(da_shutdown_callback_);
  VENUS_SWAP_FIELD_WITH_RHS(da_render_callback_);
}

VeResult SceneApp::init(DisplayApp::Config display_app_config) {
  da_startup_callback_ = [&](DisplayApp &app) -> VeResult {
    VENUS_RETURN_BAD_RESULT(venus::engine::GraphicsEngine::Config()
                                .setSynchronization2()
                                .setDynamicRendering()
                                .setBindless()
                                .init(app.display()));

    VENUS_RETURN_BAD_RESULT(venus::engine::GraphicsEngine::startup());

    if (this->startup_callback_)
      VENUS_RETURN_BAD_RESULT(this->startup_callback_(*this));

    VENUS_ASSIGN_OR_RETURN_BAD_RESULT(renderer_,
                                      venus::app::Renderer::Config().create());
    return VeResult::noError();
  };

  da_shutdown_callback_ = [&]() -> VeResult {
    vkDeviceWaitIdle(**venus::engine::GraphicsEngine::device());
    renderer_.destroy();
    scene_.destroy();
    return venus::engine::GraphicsEngine::shutdown();
  };

  da_render_callback_ =
      [&](const io::DisplayLoop::Iteration::Frame &frame) -> VeResult {
    HERMES_UNUSED_VARIABLE(frame);
    auto &gd = venus::engine::GraphicsEngine::device();
    VENUS_RETURN_BAD_RESULT(
        gd.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT));

    venus::scene::DrawContext ctx;

    VENUS_RETURN_BAD_RESULT(renderer_.begin());

    // update renderer
    venus::engine::GraphicsEngine::Globals::Types::SceneData scene_data;
    if (!selected_camera_.empty()) {
      // update camera
      auto clip_size = gd.swapchain().imageExtent();
      auto *camera_node =
          scene_.graph().get<scene::graph::CameraNode>(selected_camera_);
      HERMES_CHECK(camera_node);
      if (camera_node) {
        auto camera = camera_node->camera();
        if (camera) {
          camera->resize(clip_size.width, clip_size.height);

          scene_data.view =
              hermes::math::transpose(camera->viewTransform().matrix());
          scene_data.proj =
              hermes::math::transpose(camera->projectionTransform().matrix());
          scene_data.viewproj = scene_data.proj * scene_data.view;
        }
      }
    }

    VENUS_RETURN_BAD_RESULT(renderer_.update(scene_data));

    if (update_scene_callback_)
      VENUS_RETURN_BAD_RESULT(update_scene_callback_(scene_));

    scene_.graph().draw({}, ctx);
    renderer_.draw(ctx.objects);

    VENUS_RETURN_BAD_RESULT(renderer_.end());

    engine::GraphicsEngine::globals().ui.newFrame();
    ImGui::ShowDemoWindow();

    ImGui::Begin("Stats");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                frame.last_frame_duration.count() / 1000.f,
                1000000. / frame.current_fps_period.count());
    ImGui::End();

    engine::GraphicsEngine::globals().ui.draw();

    VENUS_RETURN_BAD_RESULT(venus::engine::GraphicsEngine::device().finish());

    return VeResult::noError();
  };

  display_app_ =
      display_app_config.setRenderFn(da_render_callback_)
          .setStartupFn(da_startup_callback_)
          .setRenderFn(da_render_callback_)
          .setShutdownFn(da_shutdown_callback_)
          .setMouseButtonFn([&](ui::Action action, ui::MouseButton button,
                                ui::Modifier modifiers) {
            HERMES_UNUSED_VARIABLE(modifiers);
            camera_controller_.mouseButton(
                action, button, engine::GraphicsEngine::display()->cursorNDC());
          })
          .setCursorPosFn([&](const hermes::geo::point2 &sc) {
            HERMES_UNUSED_VARIABLE(sc);
            camera_controller_.mouseMove(
                engine::GraphicsEngine::display()->cursorNDC());
          })
          .setMouseScrollFn([&](const hermes::geo::vec2 &d) {
            camera_controller_.mouseScroll(
                engine::GraphicsEngine::display()->cursorNDC(), d);
          })
          .setKeyFn([](ui::Action action, ui::Key key, ui::Modifier modifiers) {
            HERMES_UNUSED_VARIABLE(action);
            HERMES_UNUSED_VARIABLE(key);
            HERMES_UNUSED_VARIABLE(modifiers);
          })
          .setFPS(60)
          .setDurationInFrames(0)
          .create();

  return VeResult::noError();
}

Scene &SceneApp::scene() { return scene_; }

void SceneApp::selectCamera(const std::string &label) {
  auto *camera_node = scene_.graph().get<scene::graph::CameraNode>(label);
  if (camera_node) {
    auto camera = camera_node->camera();
    if (camera) {
      selected_camera_ = label;
      camera_controller_.setCamera(camera);
      camera_controller_.addControl(ui::MouseButton::LEFT,
                                    ui::CameraController::ControlType::Z);
      camera_controller_.addControl(ui::MouseButton::RIGHT,
                                    ui::CameraController::ControlType::ORBIT);
    }
  }
}

i32 SceneApp::run() { return display_app_.run(); }

} // namespace venus::app
