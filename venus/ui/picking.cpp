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
/// \date   2026-09-04
/// \brief

#include <venus/engine/graphics_engine.h>
#include <venus/scene/materials.h>
#include <venus/ui/picking.h>

#include <array>
#include <cstring>
#include <limits>

namespace venus::ui {

Picking::Config &Picking::Config::setImageSize(const VkExtent2D &size) {
  size_ = size;
  return *this;
}

Result<Picking> Picking::Config::build(const core::Device &device) {
  Picking picking;

  VENUS_RETURN_BAD_RESULT(picking.resize(device, size_));

  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      picking.readback_buffer_, mem::AllocatedBuffer::Config()
                                    .setSize(sizeof(u32) * 4)
                                    .addUsage(VK_BUFFER_USAGE_TRANSFER_DST_BIT)
                                    .setHostVisible()
                                    .build(device));

  auto &globals = engine::GraphicsEngine::globals();
  VENUS_DECLARE_SHARED_PTR_FROM_RESULT_OR_RETURN_BAD_RESULT(
      scene::Material::Instance, instance,
      scene::materials::MAT_Picking().write(globals.descriptors.allocator(),
                                            &globals.materials.picking));
  picking.picking_materials_.emplace(3 * sizeof(f32), instance);

  return Result<Picking>(std::move(picking));
}

VeResult Picking::resize(const core::Device &device, const VkExtent2D &size) {
  if (!size.width || !size.height)
    return VeResult::inputError();
  if (picking_image_ && depth_image_ && picking_image_view_ &&
      depth_image_view_ && size_.width == size.width &&
      size_.height == size.height)
    return VeResult::noError();

  // Build a complete replacement before releasing the current targets. A
  // failed allocation must not make a later same-size resize look successful.
  mem::AllocatedImage new_picking_image;
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      new_picking_image, mem::AllocatedImage::Config()
                             .setImageType(VK_IMAGE_TYPE_2D)
                             .setExtent({size.width, size.height, 1})
                             .setMipLevels(1)
                             .setArrayLayers(1)
                             .setFormat(VK_FORMAT_R32G32B32A32_UINT)
                             .setTiling(VK_IMAGE_TILING_OPTIMAL)
                             .setInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
                             .addUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
                             .addUsage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
                             .setSamples(VK_SAMPLE_COUNT_1_BIT)
                             .setSharingMode(VK_SHARING_MODE_EXCLUSIVE)
                             .build(device));

  mem::Image::View new_picking_image_view;
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      new_picking_image_view,
      mem::Image::View::Config()
          .setFormat(VK_FORMAT_R32G32B32A32_UINT)
          .setViewType(VK_IMAGE_VIEW_TYPE_2D)
          .setSubresourceRange({VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1})
          .build(new_picking_image));

  mem::AllocatedImage new_depth_image;
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      new_depth_image,
      mem::AllocatedImage::Config::forDepthBuffer(size).build(device));

  mem::Image::View new_depth_image_view;
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      new_depth_image_view,
      mem::Image::View::Config()
          .setFormat(VK_FORMAT_D32_SFLOAT)
          .setViewType(VK_IMAGE_VIEW_TYPE_2D)
          .setSubresourceRange({VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1})
          .build(new_depth_image));

  picking_image_view_ = std::move(new_picking_image_view);
  depth_image_view_ = std::move(new_depth_image_view);
  picking_image_ = std::move(new_picking_image);
  depth_image_ = std::move(new_depth_image);
  size_ = size;
  request_pending_ = false;
  readback_pending_ = false;
  return VeResult::noError();
}

VeResult Picking::request(VkOffset2D pixel) {
  if (pixel.x < 0 || pixel.y < 0 || static_cast<u32>(pixel.x) >= size_.width ||
      static_cast<u32>(pixel.y) >= size_.height)
    return VeResult::outOfBounds();
  requested_pixel_ = pixel;
  request_pending_ = true;
  return VeResult::noError();
}

VeResult Picking::collectResult() {
  if (!readback_pending_)
    return VeResult::noError();

  VENUS_DECLARE_OR_RETURN_BAD_RESULT(void *, mapped, readback_buffer_.map());
  auto invalidate_result = readback_buffer_.invalidate(sizeof(u32) * 4);
  if (invalidate_result != VeResult::noError()) {
    readback_buffer_.unmap();
    return invalidate_result;
  }
  std::array<u32, 4> pixel{};
  std::memcpy(pixel.data(), mapped, sizeof(pixel));
  readback_buffer_.unmap();

  constexpr u32 no_hit = std::numeric_limits<u32>::max();
  if (pixel[0] == no_hit)
    result_ = PickResult{};
  else
    result_ = PickResult{true, pixel[0], pixel[1]};
  readback_pending_ = false;
  return VeResult::noError();
}

std::optional<Picking::PickResult> Picking::takeResult() {
  auto result = std::move(result_);
  result_.reset();
  return result;
}

VeResult Picking::pick(const pipeline::CommandBuffer &cb,
                       const scene::RasterContext &ctx,
                       const hermes::geo::transform3f &view_projection) {
  if (!request_pending_)
    return VeResult::noError();

  VkClearValue clear_value = {};
  clear_value.color.uint32[0] = std::numeric_limits<u32>::max();
  VkClearValue depth_clear_value{};
  depth_clear_value.depthStencil.depth = 0.f;

  cb.transitionImage(*picking_image_, VK_IMAGE_LAYOUT_UNDEFINED,
                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  cb.transitionImage(*depth_image_, VK_IMAGE_LAYOUT_UNDEFINED,
                     VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

  auto rendering_info =
      pipeline::CommandBuffer::RenderingInfo()
          .setLayerCount(1)
          .setRenderArea({requested_pixel_, VkExtent2D{1, 1}})
          .addColorAttachment(
              pipeline::CommandBuffer::RenderingInfo::Attachment()
                  .setImageLayout(VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL)
                  .setImageView(*picking_image_view_)
                  .setStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
                  .setLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
                  .setClearValue(clear_value))
          .setDepthAttachment(
              pipeline::CommandBuffer::RenderingInfo::Attachment()
                  .setImageLayout(VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
                  .setImageView(*depth_image_view_)
                  .setStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE)
                  .setLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
                  .setClearValue(depth_clear_value));

  cb.beginRendering(*rendering_info);

  cb.setViewport(static_cast<f32>(size_.width), static_cast<f32>(size_.height),
                 0.f, 1.f);

  // Only one pixel contributes to the result. Keep the full viewport so the
  // scene projection remains unchanged, while limiting raster work to the
  // requested pixel.
  cb.setScissor(requested_pixel_.x, requested_pixel_.y, 1, 1);

  for (u32 object_index = 0; object_index < ctx.objects.size();
       ++object_index) {
    const auto &o = ctx.objects[object_index];
    if (!o.vertex_buffer || (o.indexed && !o.index_buffer) ||
        o.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
      continue;

    // Picking reads only a float3 position at offset zero. Use a matching
    // pipeline stride so interleaved meshes remain pickable.
    bool has_compatible_position = false;
    for (const auto &component : o.vertex_layout.components()) {
      if (component.type == mem::VertexLayout::ComponentType::Position &&
          component.format == VK_FORMAT_R32G32B32_SFLOAT &&
          component.offset == 0 &&
          o.vertex_layout.stride() >= 3 * sizeof(f32)) {
        has_compatible_position = true;
        break;
      }
    }
    if (!has_compatible_position)
      continue;

    auto material_it = picking_materials_.find(o.vertex_layout.stride());
    if (material_it == picking_materials_.end()) {
      auto &globals = engine::GraphicsEngine::globals();
      VENUS_DECLARE_SHARED_PTR_FROM_RESULT_OR_RETURN_BAD_RESULT(
          scene::Material, material,
          scene::materials::MAT_Picking::material(
              engine::GraphicsEngine::device(), o.vertex_layout.stride()));
      VENUS_DECLARE_SHARED_PTR_FROM_RESULT_OR_RETURN_BAD_RESULT(
          scene::Material::Instance, instance,
          scene::materials::MAT_Picking().write(globals.descriptors.allocator(),
                                                material));
      material_it =
          picking_materials_.emplace(o.vertex_layout.stride(), instance).first;
    }
    const auto &picking_material = material_it->second;
    cb.bindPipeline(*picking_material->pipeline(),
                    VK_PIPELINE_BIND_POINT_GRAPHICS);

    cb.bindVertexBuffers(0, {o.vertex_buffer}, {0});

    if (o.indexed && o.index_buffer)
      cb.bindIndexBuffer(o.index_buffer, 0, o.index_type);

    engine::GraphicsEngine::Globals::Types::PickingPushConstants pc{};
    pc.mvp = hermes::geo::transform3f(
        hermes::math::transpose((view_projection * o.transform).matrix()));
    pc.object_id = object_index;
    pc.first_vertex = o.first_vertex;

    if (o.indexed && o.index_buffer != VK_NULL_HANDLE) {
      const u32 primitive_count = o.count / 3;
      for (u32 primitive_index = 0; primitive_index < primitive_count;
           ++primitive_index) {
        pc.primitive_id = primitive_index;
        pc.derive_primitive_id = 0;
        cb.pushConstants(
            *picking_material->pipelineLayout(),
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
            sizeof(
                engine::GraphicsEngine::Globals::Types::PickingPushConstants),
            &pc);
        cb.drawIndexed(3, 1, o.first_index + primitive_index * 3,
                       o.vertex_offset, 0);
      }
    } else {
      pc.derive_primitive_id = 1;
      cb.pushConstants(
          *picking_material->pipelineLayout(),
          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
          sizeof(engine::GraphicsEngine::Globals::Types::PickingPushConstants),
          &pc);
      cb.draw(o.count, 1, o.first_vertex, 0);
    }
  }

  cb.endRendering();

  cb.transitionImage(*picking_image_, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                     VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  VkBufferImageCopy copy_region{};
  copy_region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
  copy_region.imageOffset = {requested_pixel_.x, requested_pixel_.y, 0};
  copy_region.imageExtent = {1, 1, 1};
  cb.copy(*picking_image_, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
          *readback_buffer_, {copy_region});
  cb.makeBufferHostReadable(*readback_buffer_);

  request_pending_ = false;
  readback_pending_ = true;

  return VeResult::noError();
}

void Picking::destroy() noexcept {
  picking_image_view_.destroy();
  depth_image_view_.destroy();
  picking_image_.destroy();
  depth_image_.destroy();
  readback_buffer_.destroy();
  for (auto &item : picking_materials_)
    item.second.destroy();
  picking_materials_.clear();
  request_pending_ = false;
  readback_pending_ = false;
  result_.reset();
}

} // namespace venus::ui
