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

#include <venus/engine/graphics_engine.h>
#include <venus/utils/macros.h>
#include <venus/utils/vk_debug.h>

#include <imgui.h>

#define RASTERIZER_GLOBAL_DESCRITOR_BUFFER_NAME                                \
  "scene_app_global_descriptor_data"

namespace venus::app {

template <>
Result<SceneApp> SceneApp::Setup<SceneApp::Config, SceneApp>::build() const {
  SceneApp app;

  // scene app configuration
  app.update_scene_callback_ = update_scene_callback_;
  // display app callbacks are created later
  // app.startup_callback_ = ...;
  // app.update_scene_callback_ = ...;
  app.sa_shutdown_callback_ = shutdown_callback_;
  app.sa_render_callback_ = render_callback_;
  app.sa_startup_callback_ = startup_callback_;

  app.window_ = display_;
  VENUS_RETURN_BAD_RESULT(app.window_->init(title_.c_str(), resolution_));
  app.window_->key_func = key_func_;
  app.window_->mouse_button_func = mouse_button_func_;
  app.window_->cursor_pos_func = cursor_pos_func_;
  app.window_->scroll_func = scroll_func_;

  app.fps_ = fps_;
  app.frames_ = frames_;

  app.renderer_ = engine::SceneRasterizer::Ptr::shared();

  return Result<SceneApp>(std::move(app));
}

SceneApp::~SceneApp() noexcept { destroy(); }

SceneApp::SceneApp(SceneApp &&rhs) noexcept { *this = std::move(rhs); }

SceneApp &SceneApp::operator=(SceneApp &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void SceneApp::destroy() noexcept {
  scene_.destroy();
  global_descriptor_set_.destroy();
  descriptor_allocator_.destroy();
  renderer_.destroy();
  DisplayApp::destroy();
}

void SceneApp::swap(SceneApp &rhs) {
  VENUS_SWAP_FIELD_WITH_RHS(scene_);
  VENUS_SWAP_FIELD_WITH_RHS(sa_startup_callback_);
  VENUS_SWAP_FIELD_WITH_RHS(sa_shutdown_callback_);
  VENUS_SWAP_FIELD_WITH_RHS(sa_render_callback_);
  VENUS_SWAP_FIELD_WITH_RHS(descriptor_allocator_);
  VENUS_SWAP_FIELD_WITH_RHS(global_descriptor_set_);
  VENUS_SWAP_FIELD_WITH_RHS(renderer_);
  DisplayApp::swap(static_cast<DisplayApp &>(rhs));
}

VeResult SceneApp::init() {
  // setup display app startup callback
  startup_callback_ = [&](DisplayApp &app) -> VeResult {
    VENUS_RETURN_BAD_RESULT(venus::engine::GraphicsEngine::Config()
                                .setSynchronization2()
                                .setDynamicRendering()
                                .setRayTracing()
                                .setBindless()
                                .init(app.display()));

    VENUS_RETURN_BAD_RESULT(venus::engine::GraphicsEngine::startup());

    if (this->sa_startup_callback_)
      VENUS_RETURN_BAD_RESULT(this->sa_startup_callback_(*this));

    // global descriptor
    //
    auto &gd = engine::GraphicsEngine::device();
    auto &cache = engine::GraphicsEngine::cache();

    VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
        descriptor_allocator_,
        pipeline::DescriptorAllocator::Config()
            .setInitialSetCount(1)
            .addDescriptorType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3.f)
            .addDescriptorType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3.f)
            .build(**gd));

    VENUS_RETURN_BAD_RESULT(cache.buffers().addBuffer(
        RASTERIZER_GLOBAL_DESCRITOR_BUFFER_NAME,
        mem::AllocatedBuffer::Config::forUniform(
            sizeof(engine::GraphicsEngine::Globals::Types::SceneData)),
        *gd));

    VENUS_DECLARE_OR_RETURN_BAD_RESULT(
        auto, buffer_index,
        cache.buffers().allocate(RASTERIZER_GLOBAL_DESCRITOR_BUFFER_NAME));
    HERMES_UNUSED_VARIABLE(buffer_index);

    return VeResult::noError();
  };

  // setup display app shutdown callback
  shutdown_callback_ = [&]() -> VeResult {
    vkDeviceWaitIdle(**venus::engine::GraphicsEngine::device());
    if (this->sa_shutdown_callback_)
      VENUS_RETURN_BAD_RESULT(this->sa_shutdown_callback_());
    scene_.destroy();
    global_descriptor_set_.destroy();
    descriptor_allocator_.destroy();
    return venus::engine::GraphicsEngine::shutdown();
  };

  // setup display app render callback
  render_callback_ =
      [&](const engine::FrameLoop::Iteration::Frame &frame) -> VeResult {
    if (this->sa_render_callback_)
      VENUS_RETURN_BAD_RESULT(this->sa_render_callback_(frame));
    auto &gd = venus::engine::GraphicsEngine::device();
    VENUS_RETURN_BAD_RESULT(
        gd.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT));

    if (update_scene_callback_)
      VENUS_RETURN_BAD_RESULT(update_scene_callback_(scene_));

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
          scene_data.eye = camera->position();
        }
      }
    }

    {
      // update global descriptor
      descriptor_allocator_.reset();

      auto &cache = engine::GraphicsEngine::cache();

      VENUS_RETURN_BAD_RESULT(
          cache.buffers().copyBlock(RASTERIZER_GLOBAL_DESCRITOR_BUFFER_NAME, 0,
                                    &scene_data, sizeof(scene_data)));

      VENUS_DECLARE_OR_RETURN_BAD_RESULT(
          VkBuffer, vk_global_data_buffer,
          cache.buffers()[RASTERIZER_GLOBAL_DESCRITOR_BUFFER_NAME]);

      VkDescriptorSetVariableDescriptorCountAllocateInfo alloc_array_info{};
      alloc_array_info.sType =
          VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
      alloc_array_info.pNext = nullptr;

      u32 descriptor_counts = cache.textures().size();
      alloc_array_info.pDescriptorCounts = &descriptor_counts;
      alloc_array_info.descriptorSetCount = 1;

      VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
          global_descriptor_set_,
          descriptor_allocator_.allocate(
              engine::GraphicsEngine::globals().descriptors.scene_data_layout,
              &alloc_array_info));

      pipeline::DescriptorWriter()
          .writeBuffer(
              0, vk_global_data_buffer,
              sizeof(engine::GraphicsEngine::Globals::Types::SceneData), 0,
              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
          .writeImages(1, *cache.textures(),
                       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
          .update(global_descriptor_set_);
    }

    renderer_->render(scene_.graph(), *global_descriptor_set_);

    // render ui

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

  window_->mouse_button_func = [&](ui::Action action, ui::MouseButton button,
                                   ui::Modifier modifiers) {
    HERMES_UNUSED_VARIABLE(modifiers);
    camera_controller_.mouseButton(
        action, button, engine::GraphicsEngine::display()->cursorNDC());
  };
  window_->cursor_pos_func = [&](const hermes::geo::point2 &sc) {
    HERMES_UNUSED_VARIABLE(sc);
    camera_controller_.mouseMove(
        engine::GraphicsEngine::display()->cursorNDC());
  };
  window_->scroll_func = [&](const hermes::geo::vec2 &d) {
    camera_controller_.mouseScroll(
        engine::GraphicsEngine::display()->cursorNDC(), d);
  };
  window_->key_func = [](ui::Action action, ui::Key key,
                         ui::Modifier modifiers) {
    HERMES_UNUSED_VARIABLE(action);
    HERMES_UNUSED_VARIABLE(key);
    HERMES_UNUSED_VARIABLE(modifiers);
  };

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

i32 SceneApp::run() {
  VENUS_RETURN_ON_BAD_RESULT(init(), -1);
  return DisplayApp::run();
}

} // namespace venus::app
