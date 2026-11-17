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
  camera_controllers_.clear();
  DisplayApp::destroy();
}

void SceneApp::swap(SceneApp &rhs) {
  VENUS_SWAP_FIELD_WITH_RHS(scene_);
  VENUS_SWAP_FIELD_WITH_RHS(update_scene_callback_);
  VENUS_SWAP_FIELD_WITH_RHS(sa_shutdown_callback_);
  VENUS_SWAP_FIELD_WITH_RHS(sa_render_callback_);
  VENUS_SWAP_FIELD_WITH_RHS(ge_config_);
  VENUS_SWAP_FIELD_WITH_RHS(selected_camera_);
  VENUS_SWAP_FIELD_WITH_RHS(camera_controllers_);
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
    auto begin_result = gd.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    if (begin_result == VeResult(VeResult::Type::SwapchainNeedsResize))
      // if the swapchain needed resize we just skip this rendering phase
      return VeResult::noError();
    else if (begin_result != VeResult::noError()) {
      VENUS_RETURN_BAD_RESULT(begin_result);
    }

    if (update_scene_callback_)
      VENUS_RETURN_BAD_RESULT(update_scene_callback_(scene_, frame));

    VENUS_RETURN_BAD_RESULT(render(frame));

    // render ui
    engine::GraphicsEngine::globals().ui.newFrame();

    VENUS_RETURN_BAD_RESULT(this->ui());

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

  window_->input_event_func = [&](const io::InputEvent &event) {
    bool camera_is_selected = !selected_camera_.empty() &&
                              camera_controllers_.contains(selected_camera_);
    // Respect UI mouse capture so scene navigation does not also react to UI
    // interactions.
    const bool imgui_captures_mouse =
        engine::GraphicsEngine::uiWantsCaptureMouse();
    if (event.isKeyEvent()) {
      auto e = event.keyEvent();
      if (e.key == io::Key::ESCAPE) {
        window_->close();
      }
    } else if (event.isMouseMoveEvent() && camera_is_selected &&
               !imgui_captures_mouse) {
      camera_controllers_[selected_camera_].mouseMove(
          engine::GraphicsEngine::display()->cursorNDC());
    } else if (event.isMouseScrollEvent() && camera_is_selected &&
               !imgui_captures_mouse) {
      camera_controllers_[selected_camera_].mouseScroll(
          engine::GraphicsEngine::display()->cursorNDC(),
          event.mouseScroll().offset);
    } else if (event.isMouseButtonEvent() && camera_is_selected) {
      auto e = event.mouseButton();
      if (imgui_captures_mouse && e.action != io::Action::RELEASE)
        return;
      camera_controllers_[selected_camera_].mouseButton(
          e.action, e.button, engine::GraphicsEngine::display()->cursorNDC());
    }
  };

  return VeResult::noError();
}

Scene &SceneApp::scene() { return scene_; }

void SceneApp::selectCamera(const std::string &label) {
  if (camera_controllers_.find(label) == camera_controllers_.end()) {
    ui::CameraController controller;
    auto *camera_node = scene_.graph().get<scene::graph::CameraNode>(label);
    HERMES_CHECK(camera_node);
    auto camera = camera_node->camera();
    HERMES_CHECK(camera);
    controller.setCamera(camera);
    camera_controllers_.emplace(label, controller);
  }
  selected_camera_ = label;
}

void SceneApp::selectDefault2DCamera() {
  selectCamera("scene_app_default_2d_camera");
}

void SceneApp::selectDefault3DCamera() {
  selectCamera("scene_app_default_camera");
}

ui::CameraController &SceneApp::cameraController(const std::string &label) {
  if (camera_controllers_.find(label) == camera_controllers_.end()) {
    ui::CameraController controller;
    auto *camera_node = scene_.graph().get<scene::graph::CameraNode>(label);
    HERMES_CHECK(camera_node);
    auto camera = camera_node->camera();
    HERMES_CHECK(camera);
    controller.setCamera(camera);
    camera_controllers_.emplace(label, controller);
  }
  return camera_controllers_[label];
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
  app.sa_ui_callback_ = ui_callback_;

  app.window_ = display_;
  VENUS_RETURN_BAD_RESULT(app.window_->init(title_.c_str(), resolution_));
  app.window_->input_event_func = input_event_func_;

  app.fps_ = fps_;
  app.frames_ = frames_;
  app.ge_config_ = ge_config_;

  // setup default cameras
  {
    scene::Camera::Ptr default_camera_ptr = scene::Camera::Ptr::shared();
    *default_camera_ptr =
        scene::Camera::perspective(
            90, hermes::geo::transform_option_bits::right_handed |
                    hermes::geo::transform_option_bits::flip_y |
                    hermes::geo::transform_option_bits::zero_to_one)
            .setPosition({2.f, 1.f, 1.f});

    app.scene_.graph().addCamera("scene_app_default_camera",
                                 default_camera_ptr);

    app.cameraController("scene_app_default_camera")
        .addControl(io::MouseButton::LEFT,
                    ui::CameraController::ControlType::DOLLY)
        .addControl(io::MouseButton::RIGHT,
                    ui::CameraController::ControlType::ORBIT)
        .addScrollControl(ui::CameraController::ControlType::SCROLL_ORBIT);
  }
  { // 2d camera
    auto default_2d_camera_ptr = scene::Camera::Ptr::shared();
    *default_2d_camera_ptr =
        scene::Camera::orthographic(-10.f, 10.f, -10.f, 10.f)
            .setPosition(hermes::geo::point3(0, 1, 0));
    default_2d_camera_ptr->projection()->setFar(1000);
    default_2d_camera_ptr->projection()->setNear(-1000);

    app.scene_.graph().addCamera("scene_app_default_2d_camera",
                                 default_2d_camera_ptr);

    app.cameraController("scene_app_default_2d_camera")
        .addControl(io::MouseButton::LEFT,
                    ui::CameraController::ControlType::PAN)
        .addScrollControl(ui::CameraController::ControlType::ZOOM);
  }

  app.selectDefault3DCamera();

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
  picking_.destroy();
  global_descriptor_set_.destroy();
  descriptor_allocator_.destroy();
  SceneApp::destroy();
}

void RA_SceneApp::swap(RA_SceneApp &rhs) {
  VENUS_SWAP_FIELD_WITH_RHS(sa_startup_callback_);
  VENUS_SWAP_FIELD_WITH_RHS(sa_ui_callback_);
  VENUS_SWAP_FIELD_WITH_RHS(descriptor_allocator_);
  VENUS_SWAP_FIELD_WITH_RHS(global_descriptor_set_);
  VENUS_SWAP_FIELD_WITH_RHS(picking_);
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

  VENUS_DECLARE_OR_RETURN_BAD_RESULT(
      ui::Picking, picking,
      ui::Picking::Config()
          .setImageSize(gd.swapchain().imageExtent())
          .build(*gd));
  picking_ = std::move(picking);

  return VeResult::noError();
}

VeResult RA_SceneApp::render(const engine::FrameLoop::Iteration::Frame &frame) {
  HERMES_UNUSED_VARIABLE(frame);

  auto &gd = engine::GraphicsEngine::device();
  // GraphicsDevice::begin waits for the previous frame fence before entering
  // render(), so readback memory from an earlier pick is safe to access here.
  VENUS_RETURN_BAD_RESULT(picking_.collectResult());
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
    // VkDescriptorSetVariableDescriptorCountAllocateInfo
    // alloc_array_info{}; alloc_array_info.sType =
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

    // picking
    VENUS_RETURN_BAD_RESULT(picking_.resize(*gd, gd.swapchain().imageExtent()));

    pipeline::Rasterizer rasterizer;
    rasterizer.setRenderArea(gd.swapchain().imageExtent());
    auto err = std::visit(
        scene::DrawContextOverloaded{
            [&](scene::RasterContext &ctx) -> VeResult {
              VENUS_RETURN_BAD_RESULT(
                  picking_.pick(cb, ctx, push_constants_ctx.proj_view));
              for (const auto &o : ctx.objects) {
                pipeline::Rasterizer::RasterObject ro;
                ro.count = o.count;
                ro.first_index = o.first_index;
                ro.first_vertex = o.first_vertex;
                ro.vertex_offset = o.vertex_offset;
                ro.indexed = o.indexed;
                ro.index_type = o.index_type;
                ro.index_buffer = o.index_buffer;
                ro.vertex_buffer = o.vertex_buffer;
                ro.descriptor_sets =
                    o.material_instance->localDescriptorSetGroups();
                pipeline::Rasterizer::RasterMaterial rm;
                rm.vk_pipeline = *o.material_instance->pipeline(o.topology);
                rm.vk_pipeline_layout = *o.material_instance->pipelineLayout();

                // descriptor sets
                // TODO: assuming all materials have this global descriptor
                // set
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
  picking_.destroy();
  global_descriptor_set_.destroy();
  descriptor_allocator_.destroy();
  return VeResult::noError();
}

VeResult RA_SceneApp::requestPick(VkOffset2D pixel) {
  return picking_.request(pixel);
}

std::optional<ui::Picking::PickResult> RA_SceneApp::takePickResult() {
  return picking_.takeResult();
}

VeResult RA_SceneApp::ui() {
  if (sa_ui_callback_)
    return sa_ui_callback_(*this);
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
  app.window_->input_event_func = input_event_func_;

  app.fps_ = fps_;
  app.frames_ = frames_;
  app.ge_config_ = ge_config_;

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

VeResult RT_SceneApp::ui() { return VeResult::noError(); }

} // namespace venus::app
