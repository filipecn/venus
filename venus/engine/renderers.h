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

#pragma once

#include <venus/pipeline/rasterizer.h>
#include <venus/scene/scene_graph.h>

namespace venus::engine {

/// Interface for renderers.
class SceneRenderer {
public:
  using Ptr = hermes::Ref<SceneRenderer>;

  SceneRenderer() noexcept = default;
  virtual ~SceneRenderer() noexcept = default;

  /// Records the current command buffer with drawing command for the given
  /// scene
  /// \param scene Scene graph node
  /// \param vk_global_descriptor_set
  /// \note The global descriptor set is bound at the beginning of the array of
  ///       descriptor sets accessed by all render objects.
  virtual VeResult render(scene::graph::Node &scene,
                          VkDescriptorSet vk_global_descriptor_set) const = 0;
};

class SceneRasterizer : public SceneRenderer {
public:
  using Ptr = hermes::Ref<SceneRasterizer>;

  SceneRasterizer() noexcept = default;
  ~SceneRasterizer() noexcept = default;

  /// Records the current command buffer with drawing command for the given
  /// scene
  /// \param scene Scene graph node
  /// \param vk_global_descriptor_set
  /// \note The global descriptor set is bound at the beginning of the array of
  ///       descriptor sets accessed by all render objects.
  VeResult render(scene::graph::Node &scene,
                  VkDescriptorSet vk_global_descriptor_set) const override;
};

} // namespace venus::engine
