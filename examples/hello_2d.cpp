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

/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2026-08-04
/// \brief  Example on how to initialize venus.

#include <venus/app/scene_app.h>
#include <venus/engine/graphics_engine.h>
#include <venus/engine/shapes.h>
#include <venus/io/glfw_display.h>
#include <venus/scene/materials.h>
#include <venus/scene/models/cartesian_grid.h>
#include <venus/scene/models/sky_background.h>

using namespace venus;

VeResult init(app::RA_SceneApp &app) {
  auto &globals = engine::GraphicsEngine::globals();

  VENUS_DECLARE_SHARED_PTR_FROM_RESULT_OR_RETURN_BAD_RESULT(
      scene::models::SkyBackground, sky,
      scene::models::SkyBackground::Config().build(
          engine::GraphicsEngine::device()));
  VENUS_DECLARE_SHARED_PTR_FROM_RESULT_OR_RETURN_BAD_RESULT(
      scene::models::CartesianGrid, grid,
      scene::models::CartesianGrid::Config().build(
          engine::GraphicsEngine::device()));

  VENUS_DECLARE_SHARED_PTR_FROM_RESULT_OR_RETURN_BAD_RESULT(
      scene::AllocatedModel, cube_model_ptr,
      scene::AllocatedModel::Config::fromShape(
          scene::shapes::box, hermes::geo::bounds::bbox3::unit(),
          scene::shape_option_bits::none)
          .build(engine::GraphicsEngine::device()));

  // allocate ub for triangle model
  VENUS_DECLARE_SHARED_PTR_FROM_RESULT_OR_RETURN_BAD_RESULT(
      scene::Material::Instance, mat,
      scene::materials::MAT_Empty().write(globals.descriptors.allocator(),
                                          &globals.materials.empty));
  cube_model_ptr->setMaterialInstance(mat);

  app.scene().graph().addModel("sky", sky);
  app.scene().graph().addModel("cube", cube_model_ptr);
  app.scene().graph().addModel("grid", grid);

  return VeResult::noError();
}

int main() {
  VENUS_CHECK_VE_RESULT(core::vk::init());
  app::RA_SceneApp app;
  VENUS_ASSIGN_OR(
      app,
      app::RA_SceneApp::Config()
          .setGraphicsEngineConfig(engine::GraphicsEngine::Config()
                                       .setSynchronization2()
                                       .setDynamicRendering()
                                       .setShaderDemoteToHelperInvocation()
                                       .setBindless())
          .setDisplay<io::GLFW_Window>("Hello Vulkan Display App", {1024, 1024})
          .setStartupFn(init)
          .setFPS(60)
          .setDurationInFrames(0)
          .build(),
      return -1);
  return app.run();
}
