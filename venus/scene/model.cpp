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

/// \file   model.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30

#include <venus/scene/model.h>
#include <venus/utils/macros.h>
#include <venus/utils/vk_debug.h>

namespace venus::scene {
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(Model, addShape, const Model::Shape &,
                                     shapes_.emplace_back(value));

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(Model, setVertices, VkBuffer,
                                     vk_vertex_buffer_ = value);

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(Model, setIndices, VkBuffer,
                                     vk_index_buffer_ = value);
Model::Config &
Model::Config::pushVertexComponent(mem::VertexLayout::ComponentType component,
                                   VkFormat format) {
  vertex_layout_.pushComponent(component, format);
  return *this;
}

Result<Model> Model::Config::create() const {
  Model model;
  model.vk_vertex_buffer_ = vk_vertex_buffer_;
  model.vk_index_buffer_ = vk_index_buffer_;
  model.shapes_ = shapes_;
  model.vertex_layout_ = vertex_layout_;
  return Result<Model>(std::move(model));
}

const std::vector<Model::Shape> &Model::shapes() const { return shapes_; }

VkBuffer Model::vertexBuffer() const { return vk_vertex_buffer_; }

VkBuffer Model::indexBuffer() const { return vk_index_buffer_; }

} // namespace venus::scene

namespace venus {

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::scene::Model)
HERMES_PUSH_DEBUG_VK_FIELD(vk_vertex_buffer_);
HERMES_PUSH_DEBUG_VK_FIELD(vk_index_buffer_);
HERMES_TO_STRING_DEBUG_METHOD_END

} // namespace venus
