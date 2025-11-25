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

/// \file   ray_tracer.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-11-12
/// \brief  Ray tracer.

#pragma once

#include <venus/engine/graphics_engine.h>
#include <venus/mem/image.h>
#include <venus/scene/acceleration_structure.h>
#include <venus/scene/scene_graph.h>

namespace venus::pipeline {

/// \brief Ray tracing pipeline
class RayTracer {
public:
  struct TracerObject {
    // mesh
    u32 primitive_count;
    u32 transform_offset;
    u32 max_vertex;
    // data
    VkDeviceAddress vertex_data;
    VkDeviceAddress index_data;
    VkDeviceAddress transform_data;
    mem::VertexLayout vertex_layout;
  };
  struct UniformBuffer {
    hermes::geo::Transform view_inverse;
    hermes::geo::Transform proj_inverse;
  };
  VENUS_DECLARE_RAII_FUNCTIONS(RayTracer)
  void destroy() noexcept;
  void swap(RayTracer &rhs);

  /// \param area Output render image size (in pixels).
  RayTracer &setResolution(const VkExtent2D &resolution);
  /// \param tracer_object
  RayTracer &add(const TracerObject &tracer_object);
  /// Setup acceleration structures
  /// \param gd
  /// \param vk_queue
  /// \note This must be called before record
  VeResult prepare(const engine::GraphicsDevice &gd, VkQueue vk_queue);
  /// Records the given command buffer with the rendering commands so output
  /// is draw into the given image.
  /// \param cb Command buffer being recorded.
  /// \param vk_color_image
  /// \param vk_color_image_view
  HERMES_NODISCARD VeResult record(const CommandBuffer &cb,
                                   VkImage vk_color_image,
                                   VkImageView vk_color_image_view) const;

private:
  VeResult createPipeline(VkDevice vk_device);
  VeResult createShaderBindingTable(const core::Device &device);
  VeResult createDescriptorSets(VkDevice vk_device);

  VkExtent2D resolution_{};
  scene::AccelerationStructure blas_;
  scene::AccelerationStructure tlas_;
  mem::AllocatedImage image_;
  mem::Image::View image_view_;

  RayTracingPipeline pipeline_;
  Pipeline::Layout pipeline_layout_;

  mem::AllocatedBuffer raygen_sbt_;
  mem::AllocatedBuffer miss_sbt_;
  mem::AllocatedBuffer hit_sbt_;

  DescriptorSet::Layout descriptor_set_layout_;
  DescriptorAllocator descriptor_allocator_;
  DescriptorSet descriptor_set_;

  mem::AllocatedBuffer ubo_;
};

}; // namespace venus::pipeline
