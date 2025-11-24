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
/// \date   2025-06-07
/// \brief  Rasterizer.

#pragma once

#include <venus/engine/graphics_engine.h>
#include <venus/scene/scene_graph.h>

namespace venus::engine::deprecated {

class Rasterizer {
public:
  struct Config {
    Result<Rasterizer> build() const;
  };

  VENUS_DECLARE_RAII_FUNCTIONS(Rasterizer)

  void destroy() noexcept;
  void swap(Rasterizer &rhs);

  HERMES_NODISCARD VeResult begin();
  /// Update global descriptor set data
  HERMES_NODISCARD VeResult
  update(const engine::GraphicsEngine::Globals::Types::SceneData &scene_data);
  HERMES_NODISCARD VeResult end();

  /// \param render_objects List of render objects to be rendered in sequence.
  void draw(const std::vector<scene::RenderObject> &render_objects);
  /// \param render_object
  void draw(const scene::RenderObject &render_object);

  pipeline::DescriptorAllocator &descriptorAllocator();

private:
  VkPipeline last_pipeline_{nullptr};
  venus::scene::Material *last_material_{nullptr};
  VkBuffer last_index_buffer_{nullptr};
  VkBuffer last_vertex_buffer_{nullptr};

  // frame data
  //
  pipeline::DescriptorAllocator descriptor_allocator_;
  /// The global descriptor set is bound at the beginning of the array of
  /// descriptor sets accessed by all render objects.
  pipeline::DescriptorSet global_descriptor_set_;
};

} // namespace venus::engine::deprecated
