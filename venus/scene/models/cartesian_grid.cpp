/* Copyright (c) 2026, FilipeCN.
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

/// \file   cartesian_grid.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2026-02-17

#include <venus/scene/models/cartesian_grid.h>

#include <venus/engine/graphics_engine.h>
#include <venus/engine/shapes.h>

#include <hermes/geometry/transform.h>

namespace venus::scene::models {

Result<Material> createMaterial(const engine::GraphicsDevice &gd) {
  // setup shaders
  std::filesystem::path shaders_path(VENUS_SHADERS_PATH);
  pipeline::ShaderModule vert;
  pipeline::ShaderModule frag;
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      vert, pipeline::ShaderModule::Config()
                .setEntryFuncName("main")
                .fromSpvFile(shaders_path / "cartesian_grid.vert.spv")
                .build(**gd));
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      frag, pipeline::ShaderModule::Config()
                .setEntryFuncName("main")
                .fromSpvFile(shaders_path / "cartesian_grid.frag.spv")
                .build(**gd));

  auto &globals = engine::GraphicsEngine::globals();

  auto pipeline_layout_config =
      pipeline::Pipeline::Layout::Config()
          .addDescriptorSetLayout(globals.descriptors.camera_data_layout)
          .addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT |
                                    VK_SHADER_STAGE_FRAGMENT_BIT,
                                0, sizeof(hermes::geo::Transform));

  auto pipeline_config =
      pipeline::GraphicsPipeline::Config::forDynamicRendering(gd.swapchain())
          .setColorBlend(pipeline::GraphicsPipeline::ColorBlend::alphaBlend())
          .setVertexInputState(mem::VertexLayout().pushComponent(
              mem::VertexLayout::ComponentType::Position,
              VK_FORMAT_R32G32B32_SFLOAT))
          .addShaderStage(pipeline::Pipeline::ShaderStage()
                              .setStages(VK_SHADER_STAGE_VERTEX_BIT)
                              .build(vert))
          .addShaderStage(pipeline::Pipeline::ShaderStage()
                              .setStages(VK_SHADER_STAGE_FRAGMENT_BIT)
                              .build(frag));

  VENUS_DECLARE_OR_RETURN_BAD_RESULT(
      Material, m,
      Material::Config()
          .setMaterialPipelineConfig(
              Material::Pipeline::Config()
                  .setPipelineConfig(pipeline_config)
                  .setPipelineLayoutConfig(pipeline_layout_config))
          .build(**gd, *gd.renderpass()));

  return Result<Material>(std::move(m));
}

CartesianGrid::Config::Config() noexcept {
  bounds_ = hermes::geo::bounds::bbox3::unit();
  bounds_.expand(10.f);
}

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(CartesianGrid, setBounds,
                                     const hermes::geo::bounds::bbox3 &,
                                     bounds_ = value)

Result<CartesianGrid>
CartesianGrid::Config::build(const engine::GraphicsDevice &gd) const {
  CartesianGrid cg;

  // setup material
  VENUS_DECLARE_SHARED_PTR_FROM_RESULT_OR_RETURN_BAD_RESULT(Material, material,
                                                            createMaterial(gd));
  cg.material_ = material;

  // plane meshes
  hermes::geo::vec2 plane_size(10.f, 10.f);
  shape_options options = shape_option_bits::uv;
  Model::Mesh meshes[3];
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      meshes[0], shapes::plane(hermes::geo::Plane::XZ(), plane_size));
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      meshes[1], shapes::plane(hermes::geo::Plane::XY(), plane_size));
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      meshes[2], shapes::plane(hermes::geo::Plane::YZ(), plane_size));

  Model::Mesh mesh;
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(mesh, shapes::merge(meshes[0], meshes[1]));
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(mesh, shapes::merge(mesh, meshes[2]));

  // create a single buffer for all planes
  auto vertex_buffer_size = mesh.aos.dataSize();
  auto index_buffer_size = sizeof(u32) * mesh.indices.size();
  // vertices
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      cg.storage_.vertices,
      mem::AllocatedBuffer::Config ::forStorage(
          vertex_buffer_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
          .enableShaderDeviceAddress()
          .setDeviceLocal()
          .build(*gd));
  // indices
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      cg.storage_.indices,
      mem::AllocatedBuffer::Config ::forStorage(
          index_buffer_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
          .enableShaderDeviceAddress()
          .setDeviceLocal()
          .build(*gd));

  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      cg.storage_.transform,
      mem::AllocatedBuffer::Config ::forStorage(
          sizeof(hermes::geo::Transform), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
          .enableShaderDeviceAddress()
          .setDeviceLocal()
          .build(*gd));

  // copy data

  pipeline::BufferWritter buffer_writter;
  buffer_writter.addBuffer(*cg.storage_.vertices, *mesh.aos.data(),
                           static_cast<u32>(mesh.aos.dataSize()));

  buffer_writter.addBuffer(*cg.storage_.indices, mesh.indices.data(),
                           sizeof(u32) * static_cast<u32>(mesh.indices.size()));

  hermes::geo::Transform identity;
  buffer_writter.addBuffer(*cg.storage_.transform, &identity,
                           sizeof(hermes::geo::Transform));

  VENUS_RETURN_BAD_RESULT(buffer_writter.immediateSubmit(gd));

  cg.vk_vertex_buffer_ = *cg.storage_.vertices;
  cg.vk_index_buffer_ = *cg.storage_.indices;
  cg.vk_transform_buffer_ = *cg.storage_.transform;
  cg.vk_vertex_buffer_address_ = cg.storage_.vertices.deviceAddress();
  cg.vk_index_buffer_address_ = cg.storage_.indices.deviceAddress();
  cg.vk_transform_buffer_address_ = cg.storage_.transform.deviceAddress();
  cg.vertex_layout_ = meshes[0].vertex_layout;

  // setup a shape for each plane
  u32 index_base = 0;
  for (h_index i = 0; i < 3; ++i) {
    Model::Shape shape;
    shape.bounds = {};
    shape.index_count = 3; // static_cast<u32>(meshes[i].indices.size());
    shape.index_base = 0;  // index_base;
    shape.vertex_count = static_cast<u32>(meshes[i].aos.size());
    index_base += static_cast<u32>(meshes[i].indices.size());

    VENUS_DECLARE_SHARED_PTR_FROM_RESULT_OR_RETURN_BAD_RESULT(
        Material::Instance, instance,
        Material::Instance::Config()
            .setWritePushConstants(
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                [](hermes::mem::Block &block,
                   const PushConstantsContext &ctx) -> VeResult {
                  auto inv_viewproj = hermes::math::transpose(
                      (ctx.inv_view * ctx.inv_projection).matrix());
                  VENUS_RETURN_BAD_HE_RESULT(
                      block.resize(sizeof(hermes::geo::Transform)));
                  VENUS_RETURN_BAD_HE_RESULT(block.copy(&inv_viewproj));
                  return VeResult::noError();
                })
            .setMaterial(cg.material_)
            .addGlobalSetIndex(0)
            .build(venus::engine::GraphicsEngine::globals()
                       .descriptors.allocator()))
    shape.material_instance = instance;
    cg.shapes_.emplace_back(shape);
    break;
  }

  return Result<CartesianGrid>(std::move(cg));
}

CartesianGrid::CartesianGrid(CartesianGrid &&rhs) noexcept {
  *this = std::move(rhs);
}

CartesianGrid::~CartesianGrid() noexcept { destroy(); }

CartesianGrid &CartesianGrid::operator=(CartesianGrid &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void CartesianGrid::destroy() noexcept {
  storage_.vertices.destroy();
  storage_.indices.destroy();
  material_.destroy();

  vk_index_buffer_ = VK_NULL_HANDLE;
  vk_vertex_buffer_ = VK_NULL_HANDLE;
  vk_index_buffer_ = VK_NULL_HANDLE;
  vk_transform_buffer_ = VK_NULL_HANDLE;
  vk_vertex_buffer_address_ = 0;
  vk_index_buffer_address_ = 0;
  vk_transform_buffer_address_ = 0;
}

void CartesianGrid::swap(CartesianGrid &rhs) {
  VENUS_SWAP_FIELD_WITH_RHS(storage_.vertices);
  VENUS_SWAP_FIELD_WITH_RHS(storage_.indices);
  VENUS_SWAP_FIELD_WITH_RHS(shapes_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_vertex_buffer_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_index_buffer_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_vertex_buffer_address_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_index_buffer_address_);
  VENUS_SWAP_FIELD_WITH_RHS(vertex_layout_);
  VENUS_SWAP_FIELD_WITH_RHS(material_);
}

} // namespace venus::scene::models