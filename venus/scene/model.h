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

/// \file   model.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30
/// \brief  Scene model.

#pragma once

#include <hermes/geometry/bounds.h>
#include <venus/mem/layout.h>

namespace venus::scene {

// *****************************************************************************
//                                                                      Model
// *****************************************************************************

/// The model holds the device memory that contains vertex data and indices.
/// A model may consist of multiple shapes, each representing a single mesh,
/// all occupying arbitrary regions of the same buffer.
/// \note All shapes must share the same vertex layout.
/// \note The model must have a vertex layout.
/// \note This does not the data (buffers).
class Model {
public:
  /// Model surface/piece that may be treated as a separate mesh.
  struct Shape {
    u32 index_base{0};  //< where this shape starts in the index buffer.
    u32 index_count{0}; //< index count of this shape in the index buffer.
    hermes::geo::bounds::bsphere3 bounds; //< spatial bounds of this shape.
    // MaterialInstance::Ptr material;       //< material for this shape.

    VENUS_TO_STRING_FRIEND(Shape);
  };

  /// Builder for model.
  struct Config {
    /// Define the memory section of a shape.
    /// \param shape
    Config &addShape(const Shape &shape);
    /// Append a vertex component type.
    /// \note Vertex components follow the same order they are pushed.
    /// \param format
    Config &pushVertexComponent(mem::VertexLayout::ComponentType component,
                                VkFormat format);
    /// \param vertex_data Raw pointer to vertex data.
    /// \param vertex_data_size_in_bytes
    Config &setVertices(VkBuffer vk_vertex_buffer);
    /// \param index_data Raw pointer to indices.
    /// \param index_count (NOT data size in bytes)
    Config &setIndices(VkBuffer vk_index_buffer);

    Result<Model> create() const;

  private:
    VkBuffer vk_vertex_buffer_{VK_NULL_HANDLE};
    VkBuffer vk_index_buffer_{VK_NULL_HANDLE};
    std::vector<Shape> shapes_;
    mem::VertexLayout vertex_layout_;
  };

  /// \return Model shapes.
  HERMES_NODISCARD const std::vector<Shape> &shapes() const;

  VkBuffer vertexBuffer() const;
  VkBuffer indexBuffer() const;

  VENUS_TO_STRING_FRIEND(Model);

private:
  VkBuffer vk_vertex_buffer_{VK_NULL_HANDLE};
  VkBuffer vk_index_buffer_{VK_NULL_HANDLE};
  mem::VertexLayout vertex_layout_;
  std::vector<Shape> shapes_;
};

} // namespace venus::scene
