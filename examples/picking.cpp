/* Copyright (c) 2026, FilipeCN.
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

/// \file   picking.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2026-09-07

#include <imgui.h>
#include <venus/app/scene_app.h>
#include <venus/engine/graphics_engine.h>
#include <venus/engine/shapes.h>
#include <venus/io/glfw_display.h>
#include <venus/scene/materials.h>
#include <venus/scene/models/cartesian_grid.h>
#include <venus/scene/models/sky_background.h>

#include <optional>

VeResult init(venus::app::RA_SceneApp &app) {
  auto &globals = venus::engine::GraphicsEngine::globals();

  VENUS_DECLARE_SHARED_PTR_FROM_RESULT_OR_RETURN_BAD_RESULT(
      venus::scene::models::SkyBackground, sky,
      venus::scene::models::SkyBackground::Config().build(
          venus::engine::GraphicsEngine::device()));
  VENUS_DECLARE_SHARED_PTR_FROM_RESULT_OR_RETURN_BAD_RESULT(
      venus::scene::models::CartesianGrid, grid,
      venus::scene::models::CartesianGrid::Config().build(
          venus::engine::GraphicsEngine::device()));

  VENUS_DECLARE_SHARED_PTR_FROM_RESULT_OR_RETURN_BAD_RESULT(
      venus::scene::AllocatedModel, triangle_model_ptr,
      venus::scene::AllocatedModel::Config::fromShape(
          venus::scene::shapes::triangle, //
          hermes::geo::point3f(0.f, 0.f, 0.f),
          hermes::geo::point3f(0.f, 1.f, 0.f),
          hermes::geo::point3f(1.f, 0.f, 0.f),
          venus::scene::shape_option_bits::none)
          .build(venus::engine::GraphicsEngine::device()));

  // allocate ub for triangle model
  VENUS_DECLARE_SHARED_PTR_FROM_RESULT_OR_RETURN_BAD_RESULT(
      venus::scene::Material::Instance, mat,
      venus::scene::materials::MAT_Empty().write(
          globals.descriptors.allocator(), &globals.materials.empty));
  triangle_model_ptr->setMaterialInstance(mat);

  app.scene().graph().addModel("sky", sky);
  app.scene().graph().addModel("triangle", triangle_model_ptr);
  app.scene().graph().addModel("grid", grid);

  return VeResult::noError();
}

namespace {
std::optional<venus::ui::Picking::PickResult> current_pick;
bool pick_pending = false;
} // namespace

VeResult ui(venus::app::RA_SceneApp &app) {
  if (auto result = app.takePickResult()) {
    current_pick = *result;
    pick_pending = false;
  }

  if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
      !ImGui::GetIO().WantCaptureMouse) {
    const auto cursor = app.display()->cursorPos();
    const auto window_size = app.display()->resolution();
    const auto framebuffer_size =
        venus::engine::GraphicsEngine::device().swapchain().imageExtent();
    if (window_size.width && window_size.height) {
      const auto x = static_cast<i32>(cursor.x * framebuffer_size.width /
                                      window_size.width);
      const auto y = static_cast<i32>(cursor.y * framebuffer_size.height /
                                      window_size.height);
      VENUS_RETURN_BAD_RESULT(app.requestPick({x, y}));
      pick_pending = true;
    }
  }

  if (ImGui::Begin("Picking")) {
    ImGui::TextUnformatted("Click in the scene to pick a triangle.");
    if (pick_pending) {
      ImGui::TextUnformatted("Pick pending...");
    } else if (current_pick) {
      if (current_pick->hit) {
        ImGui::Text("Object index: %u", current_pick->object_index);
        ImGui::Text("Primitive index: %u", current_pick->primitive_index);
      } else {
        ImGui::TextUnformatted("No object at the picked pixel.");
      }
    } else {
      ImGui::TextUnformatted("No pick yet.");
    }
  }
  ImGui::End();
  return VeResult::noError();
}

int main() {
  VENUS_CHECK_VE_RESULT(venus::core::vk::init());
  venus::app::RA_SceneApp app;
  VENUS_ASSIGN_OR(
      app,
      venus::app::RA_SceneApp::Config()
          .setGraphicsEngineConfig(venus::engine::GraphicsEngine::Config()
                                       .setSynchronization2()
                                       .setDynamicRendering()
                                       .setShaderDemoteToHelperInvocation()
                                       .setBindless())
          .setDisplay<venus::io::GLFW_Window>("Picking Example", {1024, 1024})
          .setStartupFn(init)
          .setUIFn(ui)
          .setFPS(60)
          .setDurationInFrames(0)
          .build(),
      return -1);
  return app.run();
}
