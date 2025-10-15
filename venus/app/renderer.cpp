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

/// \file   scene.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-06-07

#include <venus/app/renderer.h>

#include <venus/engine/graphics_engine.h>

namespace venus::app {

Renderer::~Renderer() noexcept { destroy(); }

void Renderer::destroy() noexcept {}

VeResult Renderer::begin() {
  auto &gd = venus::engine::GraphicsEngine::device();
  auto &cb = gd.commandBuffer();

  VkImage image = *gd.swapchain().images()[gd.currentTargetIndex()];
  VkImageView image_view =
      *gd.swapchain().imageViews()[gd.currentTargetIndex()];
  VkImageView depth_view = *gd.swapchain().depthBufferView();

  // clear screen

  VkClearColorValue clearColor = {30.0f / 256.0f, 30.0f / 256.0f,
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

  return VeResult::noError();
}

VeResult Renderer::end() {
  auto &gd = venus::engine::GraphicsEngine::device();
  auto &cb = gd.commandBuffer();

  cb.endRendering();

  VkImage image = *gd.swapchain().images()[gd.currentTargetIndex()];
  cb.transitionImage(image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                     VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

  return VeResult::noError();
}

void Renderer::draw(
    const std::vector<scene::RenderObject> &render_objects,
    const std::vector<VkDescriptorSet> &global_descriptor_sets_) {
  for (const auto &object : render_objects) {
    draw(object, global_descriptor_sets_);
  }
}

void Renderer::draw(
    const scene::RenderObject &ro,
    const std::vector<VkDescriptorSet> &global_descriptor_sets) {
  auto &gd = venus::engine::GraphicsEngine::device();
  auto &cb = gd.commandBuffer();
  if (last_material_ != ro.material_instance->material()) {
    if (last_pipeline_ != *(ro.material_instance->pipeline())) {
      last_pipeline_ = *(ro.material_instance->pipeline());
      // bind pipeline
      cb.bind(ro.material_instance->pipeline());

      auto extent =
          venus::engine::GraphicsEngine::device().swapchain().imageExtent();

      cb.setViewport(extent.width, extent.height, 0.f, 1.f);

      cb.setScissor(0, 0, extent.width, extent.height);

      // bind global descriptor set
      if (!global_descriptor_sets.empty())
        cb.bind(VK_PIPELINE_BIND_POINT_GRAPHICS,
                *ro.material_instance->pipelineLayout(), 0,
                global_descriptor_sets);
    }
    // bind material descriptor set
    cb.bind(VK_PIPELINE_BIND_POINT_GRAPHICS,
            *ro.material_instance->pipelineLayout(),
            global_descriptor_sets.size(),
            {*ro.material_instance->descriptorSet()});
  }

  // if (ro.vertex_buffer && ro.vertex_buffer != last_vertex_buffer_) {
  //   last_vertex_buffer_ = ro.vertex_buffer;
  //   cb.bindVertexBuffers(0, {ro.vertex_buffer}, {0});
  // }
  if (ro.index_buffer && ro.index_buffer != last_index_buffer_) {
    last_index_buffer_ = ro.index_buffer;
    cb.bindIndexBuffer(ro.index_buffer, 0, VK_INDEX_TYPE_UINT32);
  }

  // compute push constants
  venus::engine::GraphicsEngine::Globals::Types::DrawPushConstants
      push_constants;
  push_constants.world_matrix = ro.transform;
  push_constants.vertex_buffer = ro.vertex_buffer_address;

  cb.pushConstants(
      *ro.material_instance->pipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0,
      sizeof(venus::engine::GraphicsEngine::Globals::Types::DrawPushConstants),
      &push_constants);

  if (ro.index_buffer != VK_NULL_HANDLE)
    cb.drawIndexed(ro.count, 1, ro.first_index, 0, 0);
  else
    cb.draw(ro.count, 1, 0, 0);
}

} // namespace venus::app
