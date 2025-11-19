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

/// \file   rasterizer.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-11-12

#include <venus/pipeline/rasterizer.h>

namespace venus::pipeline {

VENUS_DEFINE_SET_FIELD_METHOD(Rasterizer, setClearColor,
                              const VkClearColorValue &, clear_color_ = value)
VENUS_DEFINE_SET_FIELD_METHOD(Rasterizer, setRenderArea, const VkExtent2D &,
                              render_area_ = value)

Rasterizer &Rasterizer::setDynamicRendering() {
  use_dynamic_rendering_ = true;
  return *this;
}

Rasterizer &Rasterizer::add(const Rasterizer::RasterObject &object,
                            const Rasterizer::RasterMaterial &material) {

  // cache material
  u32 material_id = materials_.size();
  auto pipeline_item = material_indices_.find(material.vk_pipeline);
  if (pipeline_item != material_indices_.end()) {
    auto layout_item = pipeline_item->second.find(material.vk_pipeline_layout);
    if (layout_item != pipeline_item->second.end())
      material_id = layout_item->second;
    else {
      pipeline_item->second[material.vk_pipeline_layout] = materials_.size();
      materials_.emplace_back(material);
    }

  } else {
    material_indices_[material.vk_pipeline][material.vk_pipeline_layout] =
        materials_.size();
    materials_.emplace_back(material);
  }
  // cache object
  objects_.push_back(std::make_pair(object, material_id));
  return *this;
}

Rasterizer &Rasterizer::sortObjects() {
  std::sort(objects_.begin(), objects_.end(),
            [](const auto &a, const auto &b) { return a.second < b.second; });
  return *this;
}

VeResult Rasterizer::record(const CommandBuffer &cb, VkImage vk_image,
                            VkImageView vk_image_view,
                            VkImageView vk_depth_view) const {
  // clear screen
  VkClearValue clear_value = {};
  clear_value.color = clear_color_;
  VkImageSubresourceRange image_range = {};
  image_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  image_range.levelCount = 1;
  image_range.layerCount = 1;
  std::vector<VkImageSubresourceRange> ranges;
  ranges.emplace_back(image_range);

  cb.transitionImage(vk_image, VK_IMAGE_LAYOUT_UNDEFINED,
                     VK_IMAGE_LAYOUT_GENERAL);

  cb.clear(vk_image, VK_IMAGE_LAYOUT_GENERAL, ranges, clear_color_);

  cb.transitionImage(vk_image, VK_IMAGE_LAYOUT_GENERAL,
                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

  VkClearValue depth_clear;
  depth_clear.depthStencil.depth = 0.f;
  auto rendering_info =
      pipeline::CommandBuffer::RenderingInfo()
          .setLayerCount(1)
          .setRenderArea({VkOffset2D{0, 0}, render_area_})
          .addColorAttachment(
              pipeline::CommandBuffer::RenderingInfo::Attachment()
                  .setImageLayout(VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL)
                  .setImageView(vk_image_view)
                  .setStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
                  .setLoadOp(VK_ATTACHMENT_LOAD_OP_LOAD))
          .setDepthAttachment(
              pipeline::CommandBuffer::RenderingInfo::Attachment()
                  .setImageLayout(VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
                  .setImageView(vk_depth_view)
                  .setStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
                  .setLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
                  .setClearValue(depth_clear));

  cb.beginRendering(*rendering_info);

  VENUS_RETURN_BAD_RESULT(draw(cb));

  cb.endRendering();

  return VeResult::noError();
}

VeResult Rasterizer::draw(const CommandBuffer &cb) const {
  // cache
  VkPipeline last_pipeline = nullptr;
  VkBuffer last_index_buffer = nullptr;
  u32 last_material = materials_.size();

  for (const auto &item : objects_) {
    u32 material_id = item.second;
    HERMES_ASSERT(material_id < materials_.size());
    const auto &material = materials_[material_id];
    const auto &object = item.first;
    if (last_material != material_id) {
      if (last_pipeline != material.vk_pipeline) {
        last_pipeline = material.vk_pipeline;
        // bind pipeline
        cb.bindPipeline(material.vk_pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);

        cb.setViewport(render_area_.width, render_area_.height, 0.f, 1.f);

        cb.setScissor(0, 0, render_area_.width, render_area_.height);

        // bind global descriptor sets
        for (const auto &ds_item : material.global_descriptor_sets)
          cb.bind(VK_PIPELINE_BIND_POINT_GRAPHICS, material.vk_pipeline_layout,
                  ds_item.first, ds_item.second);
      }
      // bind material descriptor set
      for (const auto &ds_item : object.descriptor_sets)
        cb.bind(VK_PIPELINE_BIND_POINT_GRAPHICS, material.vk_pipeline_layout,
                ds_item.first, ds_item.second);
    }

    last_material = material_id;

    // if (ro.vertex_buffer && ro.vertex_buffer != last_vertex_buffer_) {
    //   last_vertex_buffer_ = ro.vertex_buffer;
    //   cb.bindVertexBuffers(0, {ro.vertex_buffer}, {0});
    // }

    if (object.index_buffer && object.index_buffer != last_index_buffer) {
      last_index_buffer = object.index_buffer;
      cb.bindIndexBuffer(object.index_buffer, 0, VK_INDEX_TYPE_UINT32);
    }

    // push constants
    if (object.push_constants.sizeInBytes()) {
      cb.pushConstants(material.vk_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT,
                       0, object.push_constants.sizeInBytes(),
                       object.push_constants.data());
    }

    if (object.index_buffer != VK_NULL_HANDLE)
      cb.drawIndexed(object.count, 1, object.first_index, 0, 0);
    else
      cb.draw(object.count, 1, 0, 0);
  }

  return VeResult::noError();
}

} // namespace venus::pipeline
