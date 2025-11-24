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

/// \file   rasterizer.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-11-12
/// \brief  Rasterization pipeline.

#pragma once

#include <hermes/storage/block.h>
#include <venus/pipeline/command_buffer.h>

namespace venus::pipeline {

/// \brief Rasterization pipeline
class Rasterizer {
public:
  struct RasterMaterial {
    // material
    VkPipeline vk_pipeline{VK_NULL_HANDLE};
    VkPipelineLayout vk_pipeline_layout{VK_NULL_HANDLE};
    /// binding -> descriptor_sets
    std::unordered_map<u32, std::vector<VkDescriptorSet>>
        global_descriptor_sets;
  };
  struct RasterObject {
    // mesh
    u32 count{0}; //< index count or vertex count
    u32 first_index{0};
    VkBuffer index_buffer{VK_NULL_HANDLE};
    VkBuffer vertex_buffer{VK_NULL_HANDLE};
    // material
    /// binding -> descriptor_sets
    std::unordered_map<u32, std::vector<VkDescriptorSet>> descriptor_sets;
    hermes::mem::Block push_constants;
  };

  /// Raster with dynamic rendering
  Rasterizer &setDynamicRendering();
  /// \param color Clear color value.
  Rasterizer &setClearColor(const VkClearColorValue &color);
  /// \param area Render area.
  Rasterizer &setRenderArea(const VkExtent2D &area);
  /// \param raster_object Object data.
  /// \param raster_material Raster material data
  /// \note Materials are considered equal by the rasterizer when both pipeline
  ///       and pipeline layouts are the same.
  Rasterizer &add(const RasterObject &raster_object,
                  const RasterMaterial &raster_material);
  /// Sorts the interna cache of objects by materials.
  /// \note This avoids binding the same objects consecutively.
  Rasterizer &sortObjects();
  /// Records the given command buffer with the rendering commands so output
  /// is draw into the given image.
  /// \param cb Command buffer being recorded.
  /// \param vk_color_image
  /// \param vk_color_image_view
  /// \param vk_depth_view
  HERMES_NODISCARD VeResult record(const CommandBuffer &cb,
                                   VkImage vk_color_image,
                                   VkImageView vk_color_image_view,
                                   VkImageView vk_depth_view) const;

private:
  VeResult draw(const CommandBuffer &cb) const;

  std::unordered_map<VkPipeline, std::unordered_map<VkPipelineLayout, u32>>
      material_indices_;
  std::vector<RasterMaterial> materials_;
  /// object, material id pairs
  std::vector<std::pair<RasterObject, u32>> objects_;
  // config
  VkExtent2D render_area_{};
  VkClearColorValue clear_color_ = {30.0f / 256.0f, 30.0f / 256.0f,
                                    134.0f / 256.0f, 0.0f};
  bool use_dynamic_rendering_{false};
};

} // namespace venus::pipeline
