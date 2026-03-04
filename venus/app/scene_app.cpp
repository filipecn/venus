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

SceneApp::~SceneApp() noexcept { destroy(); }

SceneApp::SceneApp(SceneApp &&rhs) noexcept { *this = std::move(rhs); }

SceneApp &SceneApp::operator=(SceneApp &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void SceneApp::destroy() noexcept {
  scene_.destroy();
  DisplayApp::destroy();
}

void SceneApp::swap(SceneApp &rhs) {
  VENUS_SWAP_FIELD_WITH_RHS(scene_);
  VENUS_SWAP_FIELD_WITH_RHS(sa_shutdown_callback_);
  VENUS_SWAP_FIELD_WITH_RHS(sa_render_callback_);
  VENUS_SWAP_FIELD_WITH_RHS(ge_config_);
  DisplayApp::swap(static_cast<DisplayApp &>(rhs));
}

VeResult SceneApp::setupCallbacks() {
  // setup display app startup callback
  startup_callback_ = [&](DisplayApp &app) -> VeResult {
    VENUS_RETURN_BAD_RESULT(ge_config_.init(app.display()));
    VENUS_RETURN_BAD_RESULT(venus::engine::GraphicsEngine::startup());
    VENUS_RETURN_BAD_RESULT(init());
    return VeResult::noError();
  };

  // setup display app shutdown callback
  shutdown_callback_ = [&]() -> VeResult {
    vkDeviceWaitIdle(**venus::engine::GraphicsEngine::device());
    if (this->sa_shutdown_callback_)
      VENUS_RETURN_BAD_RESULT(this->sa_shutdown_callback_());
    scene_.destroy();
    VENUS_RETURN_BAD_RESULT(shutdown());
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

    render(frame);

    // render ui

    engine::GraphicsEngine::globals().ui.newFrame();
    ImGui::ShowDemoWindow();

    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
    if (ImGui::Begin(
            "Stats", nullptr,
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav)) {
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  frame.last_frame_duration.count() / 1000.f,
                  1000000. / frame.current_fps_period.count());
    }
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
  VENUS_RETURN_ON_BAD_RESULT(setupCallbacks(), -1);
  return DisplayApp::run();
}

Result<RA_SceneApp> RA_SceneApp::Config::build() const {
  RA_SceneApp app;

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
  app.ge_config_ = ge_config_;

  return Result<RA_SceneApp>(std::move(app));
}

RA_SceneApp::~RA_SceneApp() noexcept { destroy(); }

RA_SceneApp::RA_SceneApp(RA_SceneApp &&rhs) noexcept { *this = std::move(rhs); }

RA_SceneApp &RA_SceneApp::operator=(RA_SceneApp &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void RA_SceneApp::destroy() noexcept {
  global_descriptor_set_.destroy();
  descriptor_allocator_.destroy();
  SceneApp::destroy();
}

void RA_SceneApp::swap(RA_SceneApp &rhs) {
  VENUS_SWAP_FIELD_WITH_RHS(sa_startup_callback_);
  VENUS_SWAP_FIELD_WITH_RHS(descriptor_allocator_);
  VENUS_SWAP_FIELD_WITH_RHS(global_descriptor_set_);
  SceneApp::swap(static_cast<SceneApp &>(rhs));
}

pipeline::DescriptorAllocator &RA_SceneApp::descriptorAllocator() {
  return descriptor_allocator_;
}

VeResult RA_SceneApp::init() {
  // global descriptor
  auto &gd = engine::GraphicsEngine::device();
  auto &cache = engine::GraphicsEngine::cache();

  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      descriptor_allocator_,
      pipeline::DescriptorAllocator::Config()
          .setInitialSetCount(1)
          .addDescriptorType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3.f)
          .addDescriptorType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 30.f)
          .build(**gd));

  if (this->sa_startup_callback_)
    VENUS_RETURN_BAD_RESULT(this->sa_startup_callback_(*this));

  VENUS_RETURN_BAD_RESULT(cache.buffers().addBuffer(
      RASTERIZER_GLOBAL_DESCRITOR_BUFFER_NAME,
      mem::AllocatedBuffer::Config::forUniform(
          sizeof(engine::GraphicsEngine::Globals::Types::CameraData)),
      *gd));

  VENUS_DECLARE_OR_RETURN_BAD_RESULT(
      auto, buffer_index,
      cache.buffers().allocate(RASTERIZER_GLOBAL_DESCRITOR_BUFFER_NAME));
  HERMES_UNUSED_VARIABLE(buffer_index);
  return VeResult::noError();
}

VeResult RA_SceneApp::render(const engine::FrameLoop::Iteration::Frame &frame) {
  HERMES_UNUSED_VARIABLE(frame);

  auto &gd = engine::GraphicsEngine::device();
  // update renderer context
  scene::PushConstantsContext push_constants_ctx;
  engine::GraphicsEngine::Globals::Types::CameraData camera_data;
  if (!selected_camera_.empty()) {
    // update camera
    auto clip_size = gd.swapchain().imageExtent();
    auto *camera_node =
        scene_.graph().get<scene::graph::CameraNode>(selected_camera_);
    HERMES_CHECK(camera_node);
    if (camera_node) {
      auto camera = camera_node->camera();
      if (camera) {
        camera->resize(static_cast<f32>(clip_size.width),
                       static_cast<f32>(clip_size.height));

        camera_data.view =
            hermes::math::transpose(camera->viewTransform().matrix());
        camera_data.proj =
            hermes::math::transpose(camera->projectionTransform().matrix());
        camera_data.eye = camera->position();
        // update scene context
        push_constants_ctx.eye = camera->position();
        push_constants_ctx.projection = camera->projectionTransform().matrix();
        push_constants_ctx.view = camera->viewTransform().matrix();
        push_constants_ctx.inv_projection =
            hermes::geo::inverse(camera->projectionTransform()).matrix();
        push_constants_ctx.inv_view =
            hermes::geo::inverse(camera->viewTransform()).matrix();
        push_constants_ctx.proj_view =
            push_constants_ctx.projection * push_constants_ctx.view;
      }
    }
  }
  {
    // update global descriptor
    descriptor_allocator_.reset();

    auto &cache = engine::GraphicsEngine::cache();

    VENUS_RETURN_BAD_RESULT(
        cache.buffers().copyBlock(RASTERIZER_GLOBAL_DESCRITOR_BUFFER_NAME, 0,
                                  &camera_data, sizeof(camera_data)));

    VENUS_DECLARE_OR_RETURN_BAD_RESULT(
        VkBuffer, vk_global_data_buffer,
        cache.buffers()[RASTERIZER_GLOBAL_DESCRITOR_BUFFER_NAME]);

    VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
        global_descriptor_set_,
        descriptor_allocator_.allocate(
            engine::GraphicsEngine::globals().descriptors.camera_data_layout));

    pipeline::DescriptorWriter()
        .writeBuffer(0, vk_global_data_buffer,
                     sizeof(engine::GraphicsEngine::Globals::Types::CameraData),
                     0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
        .update(global_descriptor_set_);

    // example of creating a descriptor set with arbitrary texture count
    // VkDescriptorSetVariableDescriptorCountAllocateInfo alloc_array_info{};
    // alloc_array_info.sType =
    //     VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
    // alloc_array_info.pNext = nullptr;

    // u32 descriptor_counts = static_cast<u32>(cache.textures().size());
    // alloc_array_info.pDescriptorCounts = &descriptor_counts;
    // alloc_array_info.descriptorSetCount = 1;

    // VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
    //     global_descriptor_set_,
    //     descriptor_allocator_.allocate(
    //         engine::GraphicsEngine::globals().descriptors.scene_data_layout,
    //         &alloc_array_info));

    // pipeline::DescriptorWriter()
    //     .writeBuffer(0, vk_global_data_buffer,
    //                  sizeof(engine::GraphicsEngine::Globals::Types::CameraData),
    //                  0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
    //     .writeImages(1, *cache.textures(),
    //                  VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
    //     .update(global_descriptor_set_);
  }

  {
    auto &gd = engine::GraphicsEngine::device();
    auto &cb = gd.commandBuffer();

    VENUS_DECLARE_OR_RETURN_BAD_RESULT(
        mem::Image::Handle, color_image,
        gd.swapchain().colorImageHandle(gd.currentTargetIndex()));
    auto depth_image = gd.swapchain().depthBufferImageHandle();

    scene::DrawContext draw_ctx = scene::RasterContext();
    scene_.graph().draw({}, draw_ctx);

    pipeline::Rasterizer rasterizer;
    rasterizer.setRenderArea(gd.swapchain().imageExtent());
    auto err = std::visit(
        scene::DrawContextOverloaded{
            [&](scene::RasterContext &ctx) -> VeResult {
              for (const auto &o : ctx.objects) {
                pipeline::Rasterizer::RasterObject ro;
                ro.count = o.count;
                ro.first_index = o.first_index;
                ro.index_buffer = o.index_buffer;
                ro.vertex_buffer = o.vertex_buffer;
                ro.descriptor_sets =
                    o.material_instance->localDescriptorSetGroups();
                pipeline::Rasterizer::RasterMaterial rm;
                rm.vk_pipeline = *o.material_instance->pipeline();
                rm.vk_pipeline_layout = *o.material_instance->pipelineLayout();

                // descriptor sets
                // TODO: assuming all materials have this global descriptor set
                if (o.material_instance->hasGlobalDescriptors())
                  rm.global_descriptor_sets[0] = {*global_descriptor_set_};

                // push constants
                VENUS_RETURN_BAD_RESULT(o.material_instance->writePushConstants(
                    ro.push_constants, push_constants_ctx));
                ro.push_constants_stage_flags =
                    o.material_instance->pushConstantsStageFlags();

                rasterizer.add(ro, rm);
              }
              return VeResult::noError();
            },
            [&](scene::TracerContext &ctx) -> VeResult {
              HERMES_UNUSED_VARIABLE(ctx);
              return VeResult::incompatible();
            }},
        draw_ctx);
    VENUS_RETURN_BAD_RESULT(err);

    VENUS_RETURN_BAD_RESULT(
        rasterizer.sortObjects().record(cb, color_image, depth_image));
  }
  return VeResult::noError();
}

VeResult RA_SceneApp::shutdown() {
  global_descriptor_set_.destroy();
  descriptor_allocator_.destroy();
  return VeResult::noError();
}

Result<RT_SceneApp> RT_SceneApp::Config::build() const {
  RT_SceneApp app;

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

  return Result<RT_SceneApp>(std::move(app));
}

RT_SceneApp::~RT_SceneApp() noexcept { destroy(); }

RT_SceneApp::RT_SceneApp(RT_SceneApp &&rhs) noexcept { *this = std::move(rhs); }

RT_SceneApp &RT_SceneApp::operator=(RT_SceneApp &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void RT_SceneApp::destroy() noexcept {
  ray_tracer_.destroy();
  SceneApp::destroy();
}

void RT_SceneApp::swap(RT_SceneApp &rhs) {
  VENUS_SWAP_FIELD_WITH_RHS(sa_startup_callback_);
  SceneApp::swap(static_cast<SceneApp &>(rhs));
}

VeResult RT_SceneApp::init() {
  if (this->sa_startup_callback_)
    VENUS_RETURN_BAD_RESULT(this->sa_startup_callback_(*this));

  scene::DrawContext draw_ctx = scene::TracerContext();
  scene_.graph().draw({}, draw_ctx);

  auto &gd = engine::GraphicsEngine::device();
  std::visit(
      scene::DrawContextOverloaded{
          [&](scene::RasterContext &ctx) { HERMES_UNUSED_VARIABLE(ctx); },
          [&](scene::TracerContext &ctx) {
            for (const auto &o : ctx.objects) {
              pipeline::RayTracer::TracerObject to;
              to.primitive_count = o.primitive_count;
              to.transform_offset = 0;
              to.vertex_data = o.vertex_buffer_address;
              to.index_data = o.index_buffer_address;
              to.transform_data = o.transform_buffer_address;
              to.max_vertex = o.max_vertex;
              to.vertex_layout = o.vertex_layout;
              ray_tracer_.add(to);
            }
          }},
      draw_ctx);
  ray_tracer_.setResolution(gd.swapchain().imageExtent());
  VENUS_RETURN_BAD_RESULT(ray_tracer_.prepare(gd, VK_NULL_HANDLE));

  return VeResult::noError();
}

VeResult RT_SceneApp::render(const engine::FrameLoop::Iteration::Frame &frame) {
  HERMES_UNUSED_VARIABLE(frame);

  auto &gd = engine::GraphicsEngine::device();
  auto &cb = gd.commandBuffer();

  VkImage vk_image = *gd.swapchain().images()[gd.currentTargetIndex()];
  // VkImageView vk_image_view =
  //     *gd.swapchain().imageViews()[gd.currentTargetIndex()];
  cb.transitionImage(vk_image, VK_IMAGE_LAYOUT_UNDEFINED,
                     VK_IMAGE_LAYOUT_GENERAL);
  cb.transitionImage(vk_image, VK_IMAGE_LAYOUT_GENERAL,
                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

  VENUS_RETURN_BAD_RESULT(ray_tracer_.record(cb, vk_image));

  return VeResult::noError();
}

VeResult RT_SceneApp::shutdown() {
  ray_tracer_.destroy();
  return VeResult::noError();
}

} // namespace venus::app
