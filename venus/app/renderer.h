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

/// \file   renderer.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-06-07
/// \brief  App renderer.

#pragma once

#include <venus/scene/scene_graph.h>

namespace venus::app {

class Renderer {
public:
  Renderer() = default;
  ~Renderer() noexcept;

  void destroy() noexcept;

  HERMES_NODISCARD VeResult begin();
  HERMES_NODISCARD VeResult end();

  /// \param render_objects List of render objects to be rendered in sequence.
  /// \param global_descriptor_sets Descriptor sets are bound at the beginning
  ///                               of the set of descriptor sets accessed by
  ///                               all render objects.
  void draw(const std::vector<scene::RenderObject> &render_objects,
            const std::vector<VkDescriptorSet> &global_descriptor_sets_ = {});
  /// \param render_object
  /// \param global_descriptor_sets Descriptor sets are bound at the beginning
  ///                               of the set of descriptor sets accessed by
  ///                               the render object shader.
  void draw(const scene::RenderObject &render_object,
            const std::vector<VkDescriptorSet> &global_descriptor_sets = {});

private:
  VkPipeline last_pipeline_{nullptr};
  venus::scene::Material *last_material_{nullptr};
  VkBuffer last_index_buffer_{nullptr};
  VkBuffer last_vertex_buffer_{nullptr};
  std::vector<pipeline::DescriptorSet> global_descriptor_sets_;
};

} // namespace venus::app
