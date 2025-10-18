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

#include <venus/app/display_app.h>
#include <venus/app/renderer.h>
#include <venus/app/scene.h>
#include <venus/engine/gltf_io.h>
#include <venus/engine/graphics_engine.h>
#include <venus/engine/materials.h>
#include <venus/engine/shapes.h>
#include <venus/io/glfw_display.h>
#include <venus/scene/camera.h>

venus::app::Scene scene;
venus::scene::Camera camera;
venus::app::Renderer renderer;

VeResult initScene() {
  auto &cache = venus::engine::GraphicsEngine::cache();

  VENUS_RETURN_BAD_RESULT(scene.addNewMaterial<venus::scene::Material_Test>(
      "test", venus::engine::GraphicsEngine::device()));
  VENUS_RETURN_BAD_RESULT(
      scene.addNewMaterial<venus::scene::Material_BindlessTest>(
          "bindless", venus::engine::GraphicsEngine::device()));

  VENUS_DECLARE_SHARED_PTR_FROM_RESULT_OR_RETURN_BAD_RESULT(
      venus::scene::AllocatedModel, triangle_model_ptr,
      venus::scene::AllocatedModel::Config::fromShape(
          venus::scene::shapes::triangle, //
          hermes::geo::point3(-2.f, 0.f, 0.f),
          hermes::geo::point3(-2.f, 0.5f, 0.5f),
          hermes::geo::point3(-2.f, 0.f, 0.5f),
          venus::scene::shape_option_bits::none)
          .create(venus::engine::GraphicsEngine::device()));

  venus::scene::graph::GLTF_Node::Ptr gltf;
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      gltf,
      venus::scene::graph::GLTF_Node::from(
          std::filesystem::path(VENUS_EXAMPLE_ASSETS_PATH) / "box_textured.glb",
          venus::engine::GraphicsEngine::device()));

  scene.graph().addModel("triangle", triangle_model_ptr);
  scene.graph().get("triangle")->setVisible(false);
  scene.graph().add("box", gltf);

  // allocate ub for triangle model
  u32 buffer_index = 0;
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      buffer_index, cache.buffers().allocate(
                        "ub", sizeof(venus::scene::Material_Test::Data)));
  HERMES_UNUSED_VARIABLE(buffer_index);

  camera = venus::scene::Camera::perspective(90).setPosition({2.f, 0.f, 0.f});
  camera.projection()->setNear(1).setFar(11);

  return VeResult::noError();
}

VeResult updateScene() {
  auto &gd = venus::engine::GraphicsEngine::device();
  // auto &cache = venus::engine::GraphicsEngine::cache();
  //  update camera
  auto clip_size = gd.swapchain().imageExtent();
  camera.resize(clip_size.width, clip_size.height);

  // update renderer
  venus::engine::GraphicsEngine::Globals::Types::SceneData scene_data;
  scene_data.view = hermes::math::transpose(camera.viewTransform().matrix());
  scene_data.proj =
      hermes::math::transpose(camera.projectionTransform().matrix());
  scene_data.viewproj = scene_data.proj * scene_data.view;
  VENUS_RETURN_BAD_RESULT(renderer.update(scene_data));

  // update triangle material parameters
  const auto *material = scene.getMaterial("bindless");
  venus::scene::Material_Test parameters;
  // parameters.data.projection =
  //     hermes::math::transpose(camera.projectionTransform().matrix());
  // parameters.data.view =
  //     hermes::math::transpose(camera.viewTransform().matrix());
  // parameters.resources.data_buffer_offset = 0;
  // VENUS_ASSIGN_OR_RETURN_BAD_RESULT(parameters.resources.data_buffer,
  //                                          cache.allocatedBuffers()["ub"]);
  //// write parameters to uniform buffer
  // VENUS_RETURN_BAD_RESULT(cache.allocatedBuffers().copyBlock(
  //     "ub", 0, &parameters.data, sizeof(venus::scene::Material_Test::Data)));

  // write material descriptor set
  VENUS_DECLARE_SHARED_PTR_FROM_RESULT_OR_RETURN_BAD_RESULT(
      venus::scene::Material::Instance, mat,
      parameters.write(renderer.descriptorAllocator(), material));

  // update model material
  auto *triangle_node =
      scene.graph().get<venus::scene::graph::ModelNode>("triangle");
  triangle_node->model()->setMaterial(0, mat);

  return VeResult::noError();
}

VeResult startup(venus::app::DisplayApp &app) {

  VENUS_RETURN_BAD_RESULT(venus::engine::GraphicsEngine::Config()
                              .setSynchronization2()
                              .setDynamicRendering()
                              .setBindless()
                              .init(app.display()));

  VENUS_RETURN_BAD_RESULT(venus::engine::GraphicsEngine::startup());

  // Setup Uniform Buffers

  auto &gd = venus::engine::GraphicsEngine::device();
  auto &cache = venus::engine::GraphicsEngine::cache();
  // global ub
  // test/bindless test ub
  VENUS_RETURN_BAD_RESULT(
      cache.buffers().addBuffer("ub",
                                venus::mem::AllocatedBuffer::Config::forUniform(
                                    sizeof(venus::scene::Material_Test::Data)),
                                *gd));

  VENUS_RETURN_BAD_RESULT(initScene());

  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(renderer,
                                    venus::app::Renderer::Config().create());

  return VeResult::noError();
}

VeResult render(const venus::io::DisplayLoop::Iteration::Frame &frame) {
  HERMES_UNUSED_VARIABLE(frame);
  auto &gd = venus::engine::GraphicsEngine::device();
  VENUS_RETURN_BAD_RESULT(gd.prepare());

  VENUS_RETURN_BAD_RESULT(
      gd.beginRecord(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT));

  venus::scene::DrawContext ctx;

  VENUS_RETURN_BAD_RESULT(renderer.begin());
  updateScene();
  scene.graph().draw({}, ctx);
  renderer.draw(ctx.objects);
  VENUS_RETURN_BAD_RESULT(renderer.end());

  VENUS_RETURN_BAD_RESULT(venus::engine::GraphicsEngine::device().endRecord());
  VENUS_RETURN_BAD_RESULT(venus::engine::GraphicsEngine::device().submit());
  VENUS_RETURN_BAD_RESULT(venus::engine::GraphicsEngine::device().finish());

  return VeResult::noError();
}

VeResult shutdown() {
  vkDeviceWaitIdle(**venus::engine::GraphicsEngine::device());
  renderer.destroy();
  scene.destroy();
  return venus::engine::GraphicsEngine::shutdown();
}

int main() {
  VENUS_CHECK_VE_RESULT(venus::core::vk::init());

  return venus::app::DisplayApp::Config()
      .setDisplay<venus::io::GLFW_Window>("Hello Vulkan Display App",
                                          {1024, 1024})
      .setStartupFn(startup)
      .setShutdownFn(shutdown)
      .setRenderFn(render)
      .create()
      .run();
}
