
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

/// \file   scene_renderer.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-06-07
/// \brief  Renderer interface.

#include <venus/engine/graphics_engine.h>
#include <venus/engine/renderers.h>

namespace venus::engine {

VeResult
SceneRasterizer::render(scene::graph::Node &scene,
                        VkDescriptorSet vk_global_descriptor_set) const {
  auto &gd = engine::GraphicsEngine::device();
  auto &cb = gd.commandBuffer();

  VkImage vk_image = *gd.swapchain().images()[gd.currentTargetIndex()];
  VkImageView vk_image_view =
      *gd.swapchain().imageViews()[gd.currentTargetIndex()];
  VkImageView vk_depth_view = *gd.swapchain().depthBufferView();

  scene::DrawContext ctx;
  scene.draw({}, ctx);

  pipeline::Rasterizer rasterizer;
  rasterizer.setRenderArea(gd.swapchain().imageExtent());

  for (const auto &o : ctx.objects) {
    pipeline::Rasterizer::RasterObject ro;
    ro.count = o.count;
    ro.first_index = o.first_index;
    ro.index_buffer = o.index_buffer;
    ro.vertex_buffer = o.vertex_buffer;
    ro.descriptor_sets[vk_global_descriptor_set ? 1 : 0] = {
        *o.material_instance->descriptorSet()};
    pipeline::Rasterizer::RasterMaterial rm;
    rm.vk_pipeline = *o.material_instance->pipeline();
    rm.vk_pipeline_layout = *o.material_instance->pipelineLayout();
    if (vk_global_descriptor_set)
      rm.global_descriptor_sets[0] = {vk_global_descriptor_set};
    // push constants
    engine::GraphicsEngine::Globals::Types::DrawPushConstants push_constants;
    push_constants.world_matrix = o.transform;
    push_constants.vertex_buffer = o.vertex_buffer_address;
    VENUS_HE_RETURN_BAD_RESULT(ro.push_constants.resize(
        sizeof(engine::GraphicsEngine::Globals::Types::DrawPushConstants)));
    VENUS_HE_RETURN_BAD_RESULT(ro.push_constants.copy(&push_constants));
    rasterizer.add(ro, rm);
  }

  VENUS_RETURN_BAD_RESULT(rasterizer.sortObjects().record(
      cb, vk_image, vk_image_view, vk_depth_view));

  return VeResult::noError();
}

} // namespace venus::engine
