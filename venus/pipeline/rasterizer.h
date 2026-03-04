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

#include <venus/pipeline/command_buffer.h>

#include <hermes/storage/block.h>

namespace venus::pipeline {

/// \brief Rasterization pipeline
class Rasterizer {
public:
  struct RasterMaterial {
    // material
    VkPipeline vk_pipeline{VK_NULL_HANDLE};
    VkPipelineLayout vk_pipeline_layout{VK_NULL_HANDLE};
    /// Map of descritptor set groups indexed by the first set index of
    /// the group.
    /// The descriptor sets of each group are bound sequentially in the pipeline
    /// with indices starting at the first set index (the key).
    std::unordered_map<h_index, std::vector<VkDescriptorSet>>
        global_descriptor_sets;
  };
  struct RasterObject {
    // mesh
    u32 count{0}; //< index count or vertex count
    u32 first_index{0};
    VkBuffer index_buffer{VK_NULL_HANDLE};
    VkBuffer vertex_buffer{VK_NULL_HANDLE};
    // material
    /// Map of descritptor set groups indexed by the first set index of
    /// the group.
    /// The descriptor sets of each group are bound sequentially in the pipeline
    /// with indices starting at the first set index (the key).
    std::unordered_map<h_index, std::vector<VkDescriptorSet>> descriptor_sets;
    hermes::mem::Block push_constants;
    VkShaderStageFlags push_constants_stage_flags;
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
                                   const mem::Image::Handle &color_image,
                                   const mem::Image::Handle &depth_image) const;

private:
  VeResult draw(const CommandBuffer &cb) const;

  std::unordered_map<VkPipeline, std::unordered_map<VkPipelineLayout, h_index>>
      material_indices_;
  std::vector<RasterMaterial> materials_;
  /// object, material id pairs
  std::vector<std::pair<RasterObject, h_index>> objects_;
  // config
  VkExtent2D render_area_{};
  VkClearColorValue clear_color_ = {30.0f / 256.0f, 30.0f / 256.0f,
                                    134.0f / 256.0f, 0.0f};
  bool use_dynamic_rendering_{false};
};

} // namespace venus::pipeline

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
namespace hermes {

template <> struct DebugTraits<venus::pipeline::Rasterizer::RasterMaterial> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage
  message(const venus::pipeline::Rasterizer::RasterMaterial &data) {
    DebugMessage m;
    m.addTitle("Raster Material")
        .add("vk_pipeline", VENUS_VK_HANDLE_STRING(data.vk_pipeline))
        .add("vk_pipeline_layout",
             VENUS_VK_HANDLE_STRING(data.vk_pipeline_layout))
        .add("global descriptor sets")
        .pushTab();
    for (const auto &item : data.global_descriptor_sets) {
      m.addFmt("first set index {}", item.first);
      for (auto handle : item.second)
        m.addFmt("{}", VENUS_VK_HANDLE_STRING(handle));
    }
    return m;
  }
};

template <> struct DebugTraits<venus::pipeline::Rasterizer::RasterObject> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage
  message(const venus::pipeline::Rasterizer::RasterObject &data) {
    DebugMessage m;
    m.addTitle("Raster Object")
        .add("index_buffer", VENUS_VK_HANDLE_STRING(data.index_buffer))
        .add("vertex_buffer", VENUS_VK_HANDLE_STRING(data.vertex_buffer))
        .add("count", data.count) //< index count or vertex count
        .add("first_index", data.first_index)
        .add("push constants", data.push_constants)
        .add("local descriptor sets")
        .pushTab();
    for (const auto &item : data.descriptor_sets) {
      m.addFmt("first set index {}", item.first);
      for (auto handle : item.second)
        m.addFmt("{}", VENUS_VK_HANDLE_STRING(handle));
    }
    return m;
  }
};

} // namespace hermes

#endif // VENUS_INCLUDE_DEBUG_TRAITS