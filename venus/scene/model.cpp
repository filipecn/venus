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

namespace venus {

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::scene::Model::Shape)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_FIELD(index_base)
HERMES_PUSH_DEBUG_FIELD(index_count)
HERMES_PUSH_DEBUG_HERMES_FIELD(bounds)
HERMES_PUSH_DEBUG_LINE("material: 0x{:x}",
                       object.material ? (uintptr_t)object.material.get() : 0)
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::scene::Model)
HERMES_PUSH_DEBUG_ADDRESS_FIELD(vk_vertex_buffer_)
HERMES_PUSH_DEBUG_ADDRESS_FIELD(vk_index_buffer_)
HERMES_PUSH_DEBUG_FIELD(vk_address_)
HERMES_PUSH_DEBUG_VENUS_FIELD(vertex_layout_)
HERMES_PUSH_DEBUG_ARRAY_FIELD_BEGIN(shapes_, shape)
HERMES_UNUSED_VARIABLE(shape);
HERMES_PUSH_DEBUG_VENUS_FIELD(shapes_[i])
HERMES_PUSH_DEBUG_ARRAY_FIELD_END
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::scene::AllocatedModel)
HERMES_PUSH_DEBUG_HERMES_FIELD(mesh_.aos)
HERMES_TO_STRING_DEBUG_METHOD_END

} // namespace venus

namespace venus::scene {

hermes::geo::bounds::bsphere3 Model::Mesh::computeBounds() const {
  hermes::geo::bounds::bsphere3 bounds;
  return bounds;
}

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(Model, addShape, const Model::Shape &,
                                     shapes_.emplace_back(value));

Model::Config &Model::Config::setVertices(VkBuffer vk_vertex_buffer,
                                          VkDeviceAddress vk_address) {
  vk_vertex_buffer_ = vk_vertex_buffer;
  vk_address_ = vk_address;
  return *this;
}

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(Model, setIndices, VkBuffer,
                                     vk_index_buffer_ = value);
Model::Config &
Model::Config::pushVertexComponent(mem::VertexLayout::ComponentType component,
                                   VkFormat format) {
  vertex_layout_.pushComponent(component, format);
  return *this;
}

Result<Model> Model::Config::build() const {
  Model model;
  model.vk_vertex_buffer_ = vk_vertex_buffer_;
  model.vk_index_buffer_ = vk_index_buffer_;
  model.vertex_layout_ = vertex_layout_;
  model.vk_address_ = vk_address_;
  model.shapes_ = shapes_;
  return Result<Model>(std::move(model));
}

const std::vector<Model::Shape> &Model::shapes() const { return shapes_; }

VeResult Model::setMaterial(u32 shape_index,
                            const scene::Material::Instance::Ptr &material) {
  if (shapes_.size() <= shape_index) {
    return VeResult::outOfBounds();
  }
  shapes_[shape_index].material = material;
  return VeResult::noError();
}

VkBuffer Model::vertexBuffer() const { return vk_vertex_buffer_; }

VkBuffer Model::indexBuffer() const { return vk_index_buffer_; }

VkDeviceAddress Model::deviceAddress() const { return vk_address_; }

AllocatedModel::Config AllocatedModel::Config::fromMesh(const Mesh &mesh) {
  AllocatedModel::Config config;
  config.mesh_ = mesh;
  return config;
}

Result<AllocatedModel>
AllocatedModel::Config::build(const engine::GraphicsDevice &gd) const {
  AllocatedModel model;

  if (!mesh_.aos.size()) {
    HERMES_ERROR("Failed to create empty allocated model.");
    return VeResult::inputError();
  }

  auto vertex_buffer_size = mesh_.aos.dataSize();
  auto index_buffer_size = sizeof(u32) * mesh_.indices.size();

  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      model.storage_.vertices,
      mem::AllocatedBuffer::Config ::forStorage(
          vertex_buffer_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
          .enableShaderDeviceAddress()
          .setDeviceLocal()
          .build(*gd));

  if (index_buffer_size) {
    VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
        model.storage_.indices,
        mem::AllocatedBuffer::Config ::forStorage(
            index_buffer_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
            .enableShaderDeviceAddress()
            .setDeviceLocal()
            .build(*gd));
  }

  // copy data

  pipeline::BufferWritter buffer_writter;
  buffer_writter.addBuffer(*model.storage_.vertices, *mesh_.aos.data(),
                           vertex_buffer_size);

  if (index_buffer_size) {
    buffer_writter.addBuffer(*model.storage_.indices, mesh_.indices.data(),
                             index_buffer_size);
  }

  VENUS_RETURN_BAD_RESULT(buffer_writter.immediateSubmit(gd));

  model.mesh_ = mesh_;
  model.vk_vertex_buffer_ = *model.storage_.vertices;
  model.vk_index_buffer_ = *model.storage_.indices;
  model.vk_address_ = model.storage_.vertices.deviceAddress();

  // setup single shape for whole model

  Model::Shape shape;
  shape.bounds = model.mesh_.computeBounds();
  shape.material = {};
  shape.index_count = model.mesh_.indices.size();
  shape.index_base = 0;
  shape.vertex_count = model.mesh_.aos.size();

  model.shapes_.emplace_back(shape);

  return Result<AllocatedModel>(std::move(model));
}

AllocatedModel::AllocatedModel(AllocatedModel &&rhs) noexcept { swap(rhs); }

AllocatedModel::~AllocatedModel() noexcept { destroy(); }

AllocatedModel &AllocatedModel::operator=(AllocatedModel &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void AllocatedModel::destroy() noexcept {
  storage_.vertices.destroy();
  storage_.indices.destroy();
  HERMES_CHECK_HE_RESULT(mesh_.aos.clear());
  mesh_.indices.clear();
  mesh_.layout.clear();
}

void AllocatedModel::swap(AllocatedModel &rhs) {
  VENUS_SWAP_FIELD_WITH_RHS(storage_.vertices);
  VENUS_SWAP_FIELD_WITH_RHS(storage_.indices);
  VENUS_SWAP_FIELD_WITH_RHS(mesh_);
  VENUS_SWAP_FIELD_WITH_RHS(shapes_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_vertex_buffer_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_index_buffer_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_address_);
  VENUS_SWAP_FIELD_WITH_RHS(vertex_layout_);
}

} // namespace venus::scene
