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

#include <venus/engine/graphics_device.h>
#include <venus/mem/buffer.h>
#include <venus/mem/layout.h>
#include <venus/scene/material.h>

#include <hermes/geometry/bounds.h>
#include <hermes/storage/aos.h>

namespace venus::scene {

/// The model holds the device memory that contains vertex data and indices.
/// A model may consist of multiple shapes, each representing a single mesh,
/// all occupying arbitrary regions of the same buffer.
/// \note All shapes must share the same vertex layout.
/// \note The model must have a vertex layout.
/// \note This does not the data (buffers).
class Model {
public:
  using Ptr = hermes::Ref<Model>;

  struct Mesh {
    mem::VertexLayout layout;
    hermes::mem::AoS aos;
    std::vector<u32> indices;

    hermes::geo::bounds::bsphere3 computeBounds() const;
  };

  template <typename BufferType> struct Storage {
    BufferType vertices;
    BufferType indices;
  };

  /// Model surface/piece that may be treated as a separate mesh.
  /// \note The shape can be indexed or not. If index count is zero, then vertex
  ///       count must be greater than zero. This determines how this shape
  ///       will be rendered.
  struct Shape {
    hermes::geo::bounds::bsphere3 bounds;    //< spatial bounds of this shape.
    scene::Material::Instance::Ptr material; //< material for this shape.
    u32 index_base{0};   //< where this shape starts in the index buffer.
    u32 index_count{0};  //< index count of this shape in the index buffer.
    u32 vertex_count{0}; //< if this contains an index buffer or not

    VENUS_to_string_FRIEND(Shape);
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
    Config &setVertices(VkBuffer vk_vertex_buffer, VkDeviceAddress vk_address);
    Config &setIndices(VkBuffer vk_index_buffer);

    Result<Model> create() const;

  private:
    VkBuffer vk_vertex_buffer_{VK_NULL_HANDLE};
    VkBuffer vk_index_buffer_{VK_NULL_HANDLE};
    VkDeviceAddress vk_address_{0};
    std::vector<Shape> shapes_;
    mem::VertexLayout vertex_layout_;
  };

  /// \return Model shapes.
  HERMES_NODISCARD const std::vector<Shape> &shapes() const;
  ///
  VeResult setMaterial(u32 shape_index,
                       const scene::Material::Instance::Ptr &material);
  VkBuffer vertexBuffer() const;
  VkBuffer indexBuffer() const;
  VkDeviceAddress deviceAddress() const;

protected:
  std::vector<Shape> shapes_;
  VkBuffer vk_vertex_buffer_{VK_NULL_HANDLE};
  VkBuffer vk_index_buffer_{VK_NULL_HANDLE};
  VkDeviceAddress vk_address_{0};
  mem::VertexLayout vertex_layout_;

  VENUS_to_string_FRIEND(Model);
};

class AllocatedModel : public Model {
public:
  using Ptr = hermes::Ref<AllocatedModel>;

  struct Config {
    /// Sets the model mesh from the resulting mesh a given function and its
    /// parameters.
    /// \note The generator function must return a Result<Model::Mesh> object.
    /// \tparam Func mesh generator function type
    /// \tparam Args set of parameter types for the generator function
    /// \param f mesh generator function
    /// \param args parameters passed forward to the generator function
    /// \return configuration containing the given mesh.
    template <typename Func, typename... Args>
    static Config fromShape(Func f, Args &&...args) {
      Config config;
      VENUS_ASSIGN_RESULT(config.mesh_, f(std::forward<Args>(args)...));
      return config;
    }
    /// Sets the model from the given mesh.
    /// \param mesh
    static Config fromMesh(const Mesh &mesh);
    /// Sets material_instance for all shapes in this model.
    /// \param material_instance
    Config &setMaterial(const Material::Instance::Ptr material_instance);
    /// Creates an allocated model from this configuration.
    /// \note If no shapes are defined, a single shape encompassing the whole
    ///       model is created.
    Result<AllocatedModel> create(const engine::GraphicsDevice &gd) const;

  private:
    Model::Mesh mesh_;
    Material::Instance::Ptr material_instance_;
  };

  VENUS_DECLARE_RAII_FUNCTIONS(AllocatedModel);

  void destroy() noexcept;
  void swap(AllocatedModel &rhs);

private:
  Model::Storage<mem::AllocatedBuffer> storage_;
  Model::Mesh mesh_;

  VENUS_to_string_FRIEND(AllocatedModel);
};

} // namespace venus::scene
