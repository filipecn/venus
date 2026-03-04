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

/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-06-07
/// \brief  Example on how to initialize venus.

#include <venus/app/scene_app.h>
#include <venus/engine/graphics_engine.h>
#include <venus/engine/shapes.h>
#include <venus/io/glfw_display.h>
#include <venus/scene/helpers/cartesian_grid.h>
#include <venus/scene/helpers/sky_background.h>
#include <venus/scene/materials.h>

auto camera = venus::scene::Camera::perspective(
                  90, hermes::geo::transform_option_bits::right_handed |
                          hermes::geo::transform_option_bits::flip_y |
                          hermes::geo::transform_option_bits::zero_to_one)
                  .setPosition({2.f, 1.f, 0.f});

VeResult init(venus::app::RA_SceneApp &app) {
  camera.projection()->setNear(0.1f).setFar(1000);
  app.scene().graph().addCamera("main", &camera);
  app.selectCamera("main");

  auto &globals = venus::engine::GraphicsEngine::globals();

  VENUS_DECLARE_SHARED_PTR_FROM_RESULT_OR_RETURN_BAD_RESULT(
      venus::scene::helpers::SkyBackground, sky,
      venus::scene::helpers::SkyBackground::Config().build(
          venus::engine::GraphicsEngine::device()));
  VENUS_DECLARE_SHARED_PTR_FROM_RESULT_OR_RETURN_BAD_RESULT(
      venus::scene::helpers::CartesianGrid, grid,
      venus::scene::helpers::CartesianGrid::Config().build(
          venus::engine::GraphicsEngine::device()));

  VENUS_DECLARE_SHARED_PTR_FROM_RESULT_OR_RETURN_BAD_RESULT(
      venus::scene::AllocatedModel, triangle_model_ptr,
      venus::scene::AllocatedModel::Config::fromShape(
          venus::scene::shapes::triangle, //
          hermes::geo::point3(0.f, 0.f, 0.f),
          hermes::geo::point3(0.f, 1.f, 0.f),
          hermes::geo::point3(1.f, 0.f, 0.f),
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
          .setDisplay<venus::io::GLFW_Window>("Hello Vulkan Display App",
                                              {1024, 1024})
          .setStartupFn(init)
          .setFPS(60)
          .setDurationInFrames(0)
          .build(),
      return -1);
  return app.run();
}
