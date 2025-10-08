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

venus::scene::graph::GLTF_Node::Ptr node;
venus::scene::Camera camera;
venus::pipeline::DescriptorAllocator frame_descriptors;
venus::scene::AllocatedModel triangle;
venus::scene::Material m_test;
venus::scene::Material m_bindless_test;

venus::app::Scene scene;

VeResult startup(venus::app::DisplayApp &app) {

  VENUS_RETURN_BAD_RESULT(venus::engine::GraphicsEngine::Config()
                              .setSynchronization2()
                              .setDynamicRendering()
                              .setBindless()
                              .init(app.display()));

  VENUS_RETURN_BAD_RESULT(venus::engine::GraphicsEngine::startup());

  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      frame_descriptors,
      venus::pipeline::DescriptorAllocator::Config()
          .setInitialSetCount(1)
          .addDescriptorType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3.f)
          .addDescriptorType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3.f)
          .create(**venus::engine::GraphicsEngine::device()));

  // VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
  //     node, venus::scene::GLTF_Node::from(
  //               std::filesystem::path(VENUS_EXAMPLE_ASSETS_PATH) / "box.glb",
  //               venus::engine::GraphicsEngine::device()));

  // HERMES_WARN("{}", venus::to_string(*node));

  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      m_test, venus::scene::Material_Test::material(
                  venus::engine::GraphicsEngine::device()));
  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      m_bindless_test, venus::scene::Material_Test::material(
                           venus::engine::GraphicsEngine::device()));

  // HERMES_WARN("{}", venus::to_string(m_color));

  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      triangle,
      venus::scene::AllocatedModel::Config::fromShape(
          venus::scene::shapes::triangle, //
                                          // hermes::geo::point3(0, 0, -11.f),
          // hermes::geo::point3(11.f / 6.f, 0, -11.f),
          // hermes::geo::point3(11.f / 6.f, 11.f, -11.f),
          hermes::geo::point3(-2.f, 0.f, 0.f),
          hermes::geo::point3(-2.f, 0.5f, 0.5f),
          hermes::geo::point3(-2.f, 0.f, 0.5f),
          venus::scene::shape_option_bits::none)
          .create(venus::engine::GraphicsEngine::device()));

  camera = venus::scene::Camera::perspective(90).setPosition({1.f, 0.f, 0.f});
  camera.projection()->setNear(1).setFar(11);

  // HERMES_WARN("{}", venus::to_string(triangle));

  return VeResult::noError();
}

VeResult shutdown() {
  // node->destroy();
  triangle.destroy();
  frame_descriptors.destroy();
  m_test.destroy();
  m_bindless_test.destroy();
  return venus::engine::GraphicsEngine::shutdown();
}

void draw(const venus::pipeline::CommandBuffer &cb,
          const venus::scene::RenderObject &ro,
          VkDescriptorSet scene_descriptor_set) {
  static VkPipeline last_pipeline{nullptr};
  static venus::scene::Material *last_material{nullptr};
  static VkBuffer last_index_buffer{nullptr};
  static VkBuffer last_vertex_buffer{nullptr};

  if (last_material != ro.material->material) {
    if (last_pipeline != *ro.material->material->pipeline().pipeline()) {
      last_pipeline = *ro.material->material->pipeline().pipeline();
      // bind pipeline
      cb.bind(ro.material->material->pipeline().pipeline());

      auto extent =
          venus::engine::GraphicsEngine::device().swapchain().imageExtent();

      cb.setViewport(extent.width, extent.height, 0.f, 1.f);

      cb.setScissor(0, 0, extent.width, extent.height);

      // bind global descriptor set
      cb.bind(VK_PIPELINE_BIND_POINT_GRAPHICS,
              *ro.material->material->pipeline().pipelineLayout(), 0,
              {scene_descriptor_set});
    }
    // bind material descriptor set
    cb.bind(VK_PIPELINE_BIND_POINT_GRAPHICS,
            *ro.material->material->pipeline().pipelineLayout(), 1,
            {*ro.material->descriptor_set});
  }

  if (ro.vertex_buffer && ro.vertex_buffer != last_vertex_buffer) {
    last_vertex_buffer = ro.vertex_buffer;
    cb.bindVertexBuffers(0, {ro.vertex_buffer}, {0});
  }
  if (ro.index_buffer && ro.index_buffer != last_index_buffer) {
    last_index_buffer = ro.index_buffer;
    cb.bindIndexBuffer(ro.index_buffer, 0, VK_INDEX_TYPE_UINT32);
  }

  // compute push constants
  venus::engine::GraphicsEngine::Globals::Types::DrawPushConstants
      push_constants;
  push_constants.world_matrix = ro.transform;
  push_constants.vertex_buffer = ro.vertex_buffer_address;

  cb.pushConstants(
      *ro.material->material->pipeline().pipelineLayout(),
      VK_SHADER_STAGE_VERTEX_BIT, 0,
      sizeof(venus::engine::GraphicsEngine::Globals::Types::DrawPushConstants),
      &push_constants);

  HERMES_WARN("{}", venus::to_string(ro));
  // if (ro.index_count)
  //   cb.drawIndexed(ro.index_count, 1, ro.first_index, 0, 0);
  // else
  cb.draw(12 * 3, 1, 0, 0);
}

VeResult render2(const venus::io::DisplayLoop::Iteration::Frame &frame) {
  auto &gd = venus::engine::GraphicsEngine::device();

  VENUS_RETURN_BAD_RESULT(gd.prepare());

  auto clip_size = gd.swapchain().imageExtent();
  camera.resize(clip_size.width, clip_size.height);
  // write shader parameters (uniform buffer)
  venus::scene::Material_Test parameters;
  parameters.data.projection =
      hermes::math::transpose(camera.projectionTransform().matrix());
  parameters.data.view =
      hermes::math::transpose(camera.viewTransform().matrix());
  HERMES_WARN("{}", venus::to_string(camera));
  HERMES_WARN("\nP*V{}", hermes::to_string(camera.projectionTransform() *
                                           camera.viewTransform()));

  venus::mem::AllocatedBuffer ub;
  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      ub,
      venus::mem::AllocatedBuffer::Config()
          .setBufferConfig(venus::mem::Buffer::Config::forUniform(
              sizeof(venus::scene::Material_Test::Data)))
          .setMemoryConfig(venus::mem::DeviceMemory::Config().setHostVisible())
          .create(*gd));

  VENUS_RETURN_BAD_RESULT(
      ub.copy(&parameters.data, sizeof(venus::scene::Material_Test::Data)));

  parameters.resources.data_buffer = *ub;
  parameters.resources.data_buffer_offset = 0;

  venus::scene::Material::Instance mat;
  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      mat, parameters.write(frame_descriptors, &m_bindless_test));

  VENUS_RETURN_BAD_RESULT(
      gd.beginRecord(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT));
  auto &cb = gd.commandBuffer();

  VkImage image = *gd.swapchain().images()[gd.currentTargetIndex()];
  VkImageView image_view =
      *gd.swapchain().imageViews()[gd.currentTargetIndex()];
  VkImageView depth_view = *gd.swapchain().depthBufferView();

  // clear screen

  VkClearColorValue clearColor = {164.0f / 256.0f, 30.0f / 256.0f,
                                  134.0f / 256.0f, 0.0f};
  VkClearValue clearValue = {};
  clearValue.color = clearColor;
  VkImageSubresourceRange imageRange = {};
  imageRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  imageRange.levelCount = 1;
  imageRange.layerCount = 1;
  std::vector<VkImageSubresourceRange> ranges;
  ranges.emplace_back(imageRange);

  cb.transitionImage(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

  cb.clear(*venus::engine::GraphicsEngine::device().swapchain().images()[0],
           VK_IMAGE_LAYOUT_GENERAL, ranges, clearColor);

  cb.transitionImage(image, VK_IMAGE_LAYOUT_GENERAL,
                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

  VkClearValue depth_clear;
  depth_clear.depthStencil.depth = 0.f;
  auto rendering_info =
      venus::pipeline::CommandBuffer::RenderingInfo()
          .setLayerCount(1)
          .setRenderArea({VkOffset2D{0, 0}, gd.swapchain().imageExtent()})
          .addColorAttachment(
              venus::pipeline::CommandBuffer::RenderingInfo::Attachment()
                  .setImageLayout(VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL)
                  .setImageView(image_view)
                  .setStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
                  .setLoadOp(VK_ATTACHMENT_LOAD_OP_LOAD))
          .setDepthAttachment(
              venus::pipeline::CommandBuffer::RenderingInfo::Attachment()
                  .setImageLayout(VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
                  .setImageView(depth_view)
                  .setStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
                  .setLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
                  .setClearValue(depth_clear));

  cb.beginRendering(*rendering_info);

  // bind pipeline
  cb.bind(m_bindless_test.pipeline().pipeline());

  auto extent =
      venus::engine::GraphicsEngine::device().swapchain().imageExtent();

  cb.setViewport(extent.width, extent.height, 0.f, 1.f);

  cb.setScissor(0, 0, extent.width, extent.height);
  // bind global descriptor set
  cb.bind(VK_PIPELINE_BIND_POINT_GRAPHICS, *m_test.pipeline().pipelineLayout(),
          0, {*mat.descriptor_set});

  cb.bindVertexBuffers(0, {triangle.vertexBuffer()}, {0});

  venus::scene::Material_BindlessTest::PushConstants push_constants;
  push_constants.vertex_buffer = triangle.deviceAddress();

  cb.pushConstants(*m_bindless_test.pipeline().pipelineLayout(),
                   VK_SHADER_STAGE_VERTEX_BIT, 0,
                   sizeof(venus::scene::Material_BindlessTest::PushConstants),
                   &push_constants);

  cb.draw(3, 1, 0, 0);

  cb.endRendering();

  cb.transitionImage(image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                     VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

  VENUS_RETURN_BAD_RESULT(venus::engine::GraphicsEngine::device().endRecord());
  VENUS_RETURN_BAD_RESULT(venus::engine::GraphicsEngine::device().submit());
  VENUS_RETURN_BAD_RESULT(venus::engine::GraphicsEngine::device().finish());
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(3s);
  return VeResult::noError();
}

VeResult render(const venus::io::DisplayLoop::Iteration::Frame &frame) {
  auto &gd = venus::engine::GraphicsEngine::device();

  VENUS_RETURN_BAD_RESULT(gd.prepare());

  // setup uniform buffer

  venus::engine::GraphicsEngine::Globals::Types::SceneData scene_data;
  scene_data.view = camera.viewTransform();
  scene_data.proj = camera.projectionTransform();
  scene_data.viewproj = camera.projectionTransform() * camera.viewTransform();

  venus::mem::AllocatedBuffer scene_data_buffer;
  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      scene_data_buffer,
      venus::mem::AllocatedBuffer::Config()
          .setBufferConfig(venus::mem::Buffer::Config::forUniform(
              sizeof(venus::engine::GraphicsEngine::Globals::Types::SceneData)))
          .setMemoryConfig(venus::mem::DeviceMemory::Config().setHostVisible())
          .create(*gd));

  VENUS_RETURN_BAD_RESULT(
      scene_data_buffer.copy(&scene_data, sizeof(scene_data)));

  // setup descriptor set for scene data

  venus::pipeline::DescriptorSet scene_descriptor_set;
  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      scene_descriptor_set,
      frame_descriptors.allocate(venus::engine::GraphicsEngine::globals()
                                     .descriptors.scene_data_layout));

  venus::pipeline::DescriptorWriter()
      .writeBuffer(
          0, *scene_data_buffer,
          sizeof(venus::engine::GraphicsEngine::Globals::Types::SceneData), 0,
          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
      .update(scene_descriptor_set);

  venus::scene::DrawContext ctx;
  node->draw(hermes::geo::Transform(), ctx);

  VENUS_RETURN_BAD_RESULT(
      gd.beginRecord(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT));

  auto &cb = gd.commandBuffer();

  VkImage image = *gd.swapchain().images()[gd.currentTargetIndex()];
  VkImageView image_view =
      *gd.swapchain().imageViews()[gd.currentTargetIndex()];
  VkImageView depth_view = *gd.swapchain().depthBufferView();

  // clear screen

  VkClearColorValue clearColor = {164.0f / 256.0f, 30.0f / 256.0f,
                                  34.0f / 256.0f, 0.0f};
  VkClearValue clearValue = {};
  clearValue.color = clearColor;
  VkImageSubresourceRange imageRange = {};
  imageRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  imageRange.levelCount = 1;
  imageRange.layerCount = 1;
  std::vector<VkImageSubresourceRange> ranges;
  ranges.emplace_back(imageRange);

  cb.transitionImage(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

  cb.clear(*venus::engine::GraphicsEngine::device().swapchain().images()[0],
           VK_IMAGE_LAYOUT_GENERAL, ranges, clearColor);

  cb.transitionImage(image, VK_IMAGE_LAYOUT_GENERAL,
                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

  VkClearValue depth_clear;
  depth_clear.depthStencil.depth = 0.f;
  auto rendering_info =
      venus::pipeline::CommandBuffer::RenderingInfo()
          .setLayerCount(1)
          .setRenderArea({VkOffset2D{0, 0}, gd.swapchain().imageExtent()})
          .addColorAttachment(
              venus::pipeline::CommandBuffer::RenderingInfo::Attachment()
                  .setImageLayout(VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL)
                  .setImageView(image_view)
                  .setStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
                  .setLoadOp(VK_ATTACHMENT_LOAD_OP_LOAD))
          .setDepthAttachment(
              venus::pipeline::CommandBuffer::RenderingInfo::Attachment()
                  .setImageLayout(VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
                  .setImageView(depth_view)
                  .setStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
                  .setLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
                  .setClearValue(depth_clear));

  cb.beginRendering(*rendering_info);

  for (const auto &object : ctx.objects) {
    draw(cb, object, *scene_descriptor_set);
  }

  cb.endRendering();

  cb.transitionImage(image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                     VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

  VENUS_RETURN_BAD_RESULT(venus::engine::GraphicsEngine::device().endRecord());
  VENUS_RETURN_BAD_RESULT(venus::engine::GraphicsEngine::device().submit());
  VENUS_RETURN_BAD_RESULT(venus::engine::GraphicsEngine::device().finish());
  return VeResult::noError();
}

VeResult render3(const venus::io::DisplayLoop::Iteration::Frame &frame) {
  auto &gd = venus::engine::GraphicsEngine::device();
  VENUS_RETURN_BAD_RESULT(gd.prepare());
  VENUS_RETURN_BAD_RESULT(
      gd.beginRecord(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT));
  auto &cb = gd.commandBuffer();

  venus::app::Renderer renderer;
  VENUS_RETURN_BAD_RESULT(renderer.begin());
  VENUS_RETURN_BAD_RESULT(renderer.end());

  VENUS_RETURN_BAD_RESULT(venus::engine::GraphicsEngine::device().endRecord());
  VENUS_RETURN_BAD_RESULT(venus::engine::GraphicsEngine::device().submit());
  VENUS_RETURN_BAD_RESULT(venus::engine::GraphicsEngine::device().finish());

  using namespace std::chrono_literals;
  std::this_thread::sleep_for(3s);

  return VeResult::noError();
}

int main() {
  VENUS_CHECK_VE_RESULT(venus::core::vk::init());

  return venus::app::DisplayApp::Config()
      .setDisplay<venus::io::GLFW_Window>("Hello Vulkan Display App",
                                          {1024, 1024})
      .setStartupFn(startup)
      .setShutdownFn(shutdown)
      .setRenderFn(render3)
      .create()
      .run();
}
