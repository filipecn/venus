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

#include <venus/engine/rasterizer.h>

#include <venus/engine/graphics_engine.h>

#define RASTERIZER_GLOBAL_DESCRITOR_BUFFER_NAME                                \
  "renderer_global_descriptor_data"

namespace venus::engine {

Result<Rasterizer> Rasterizer::Config::build() const {
  Rasterizer renderer;

  auto &gd = engine::GraphicsEngine::device();
  auto &cache = engine::GraphicsEngine::cache();

  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      renderer.descriptor_allocator_,
      pipeline::DescriptorAllocator::Config()
          .setInitialSetCount(1)
          .addDescriptorType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3.f)
          .addDescriptorType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3.f)
          .build(**engine::GraphicsEngine::device()));

  VENUS_RETURN_BAD_RESULT(cache.buffers().addBuffer(
      RASTERIZER_GLOBAL_DESCRITOR_BUFFER_NAME,
      mem::AllocatedBuffer::Config::forUniform(
          sizeof(engine::GraphicsEngine::Globals::Types::SceneData)),
      *gd));

  VENUS_DECLARE_OR_RETURN_BAD_RESULT(
      auto, buffer_index,
      cache.buffers().allocate(RASTERIZER_GLOBAL_DESCRITOR_BUFFER_NAME));
  HERMES_UNUSED_VARIABLE(buffer_index);

  return Result<Rasterizer>(std::move(renderer));
}

Rasterizer::Rasterizer(Rasterizer &&rhs) noexcept { *this = std::move(rhs); }

Rasterizer::~Rasterizer() noexcept { destroy(); }

Rasterizer &Rasterizer::operator=(Rasterizer &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void Rasterizer::destroy() noexcept {
  last_pipeline_ = VK_NULL_HANDLE;
  last_material_ = nullptr;
  last_index_buffer_ = VK_NULL_HANDLE;
  last_vertex_buffer_ = VK_NULL_HANDLE;
  global_descriptor_set_.destroy();
  descriptor_allocator_.destroy();
}

void Rasterizer::swap(Rasterizer &rhs) {
  VENUS_SWAP_FIELD_WITH_RHS(last_pipeline_);
  VENUS_SWAP_FIELD_WITH_RHS(last_material_);
  VENUS_SWAP_FIELD_WITH_RHS(last_index_buffer_);
  VENUS_SWAP_FIELD_WITH_RHS(last_vertex_buffer_);
  VENUS_SWAP_FIELD_WITH_RHS(global_descriptor_set_);
  VENUS_SWAP_FIELD_WITH_RHS(descriptor_allocator_);
}

VeResult Rasterizer::begin() {
  // reset
  last_pipeline_ = nullptr;
  last_material_ = nullptr;
  last_index_buffer_ = nullptr;
  last_vertex_buffer_ = nullptr;
  descriptor_allocator_.reset();

  auto &gd = engine::GraphicsEngine::device();
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

  cb.clear(image, VK_IMAGE_LAYOUT_GENERAL, ranges, clearColor);

  cb.transitionImage(image, VK_IMAGE_LAYOUT_GENERAL,
                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

  VkClearValue depth_clear;
  depth_clear.depthStencil.depth = 0.f;
  auto rendering_info =
      pipeline::CommandBuffer::RenderingInfo()
          .setLayerCount(1)
          .setRenderArea({VkOffset2D{0, 0}, gd.swapchain().imageExtent()})
          .addColorAttachment(
              pipeline::CommandBuffer::RenderingInfo::Attachment()
                  .setImageLayout(VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL)
                  .setImageView(image_view)
                  .setStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
                  .setLoadOp(VK_ATTACHMENT_LOAD_OP_LOAD))
          .setDepthAttachment(
              pipeline::CommandBuffer::RenderingInfo::Attachment()
                  .setImageLayout(VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
                  .setImageView(depth_view)
                  .setStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
                  .setLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
                  .setClearValue(depth_clear));

  cb.beginRendering(*rendering_info);

  return VeResult::noError();
}

VeResult Rasterizer::update(
    const engine::GraphicsEngine::Globals::Types::SceneData &scene_data) {

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
      .writeBuffer(0, vk_global_data_buffer,
                   sizeof(engine::GraphicsEngine::Globals::Types::SceneData), 0,
                   VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
      .writeImages(1, *cache.textures(),
                   VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
      .update(global_descriptor_set_);

  return VeResult::noError();
}

VeResult Rasterizer::end() {
  engine::GraphicsEngine::device().commandBuffer().endRendering();
  return VeResult::noError();
}

void Rasterizer::draw(const std::vector<scene::RenderObject> &render_objects) {
  for (const auto &object : render_objects) {
    draw(object);
  }
}

void Rasterizer::draw(const scene::RenderObject &ro) {
  auto &gd = engine::GraphicsEngine::device();
  auto &cb = gd.commandBuffer();
  if (last_material_ != ro.material_instance->material()) {
    if (last_pipeline_ != *(ro.material_instance->pipeline())) {
      last_pipeline_ = *(ro.material_instance->pipeline());
      // bind pipeline
      cb.bind(ro.material_instance->pipeline());

      auto extent = engine::GraphicsEngine::device().swapchain().imageExtent();

      cb.setViewport(extent.width, extent.height, 0.f, 1.f);

      cb.setScissor(0, 0, extent.width, extent.height);

      // bind global descriptor set
      cb.bind(VK_PIPELINE_BIND_POINT_GRAPHICS,
              *ro.material_instance->pipelineLayout(), 0,
              {*global_descriptor_set_});
    }
    // bind material descriptor set
    if (ro.material_instance->descriptorSet())
      cb.bind(VK_PIPELINE_BIND_POINT_GRAPHICS,
              *ro.material_instance->pipelineLayout(),
              1, // 0 - global descriptor set at the begining
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
  engine::GraphicsEngine::Globals::Types::DrawPushConstants push_constants;
  push_constants.world_matrix = ro.transform;
  push_constants.vertex_buffer = ro.vertex_buffer_address;

  cb.pushConstants(
      *ro.material_instance->pipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0,
      sizeof(engine::GraphicsEngine::Globals::Types::DrawPushConstants),
      &push_constants);

  if (ro.index_buffer != VK_NULL_HANDLE)
    cb.drawIndexed(ro.count, 1, ro.first_index, 0, 0);
  else
    cb.draw(ro.count, 1, 0, 0);
}

pipeline::DescriptorAllocator &Rasterizer::descriptorAllocator() {
  return descriptor_allocator_;
}

} // namespace venus::engine
