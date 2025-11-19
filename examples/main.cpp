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
#include <venus/io/glfw_display.h>

VeResult init(venus::app::SceneApp &app) {

  venus::scene::graph::VDB_Node::Ptr vdb;
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      vdb,
      venus::scene::graph::VDB_Node::from(
          std::filesystem::path(VENUS_EXAMPLE_ASSETS_PATH) / "box_textured.glb",
          venus::engine::GraphicsEngine::device()));

  // app.scene().graph().add("sphere", vdb);

  venus::scene::graph::GLTF_Node::Ptr gltf;
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      gltf,
      venus::scene::graph::GLTF_Node::from(
          std::filesystem::path(VENUS_EXAMPLE_ASSETS_PATH) / "box_textured.glb",
          venus::engine::GraphicsEngine::device()));

  app.scene().graph().add("box", gltf);

  auto camera = venus::scene::Camera::Ptr::shared();
  *camera = venus::scene::Camera::perspective(90).setPosition({2.f, 0.f, 0.f});
  camera->projection()->setNear(0.1).setFar(1000);
  app.scene().graph().addCamera("main", camera);
  app.selectCamera("main");

  return VeResult::noError();
}

int main() {
  VENUS_CHECK_VE_RESULT(venus::core::vk::init());

  venus::app::SceneApp app;
  VENUS_ASSIGN_OR(app,
                  venus::app::SceneApp::Config()
                      .setDisplay<venus::io::GLFW_Window>(
                          "Hello Vulkan Display App", {1024, 1024})
                      .setStartupFn(init)
                      .setFPS(60)
                      .setDurationInFrames(0)
                      .build(),
                  return -1);
  return app.run();
}
