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

/// \file   scene_graph.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30

#include <venus/scene/scene_graph.h>

#include <venus/engine/graphics_engine.h>
#include <venus/engine/shapes.h>
#include <venus/scene/materials.h>
#include <venus/utils/vk_debug.h>

#ifdef VENUS_INCLUDE_GLTF
#include <hermes/geometry/quaternion.h>

#ifdef __linux__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wredundant-move"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wtemplate-body"
#endif
#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/util.hpp>

#ifdef VENUS_INCLUDE_VDB
#define NANOVDB_USE_OPENVDB
#include <nanovdb/util/CreateNanoGrid.h>
#include <nanovdb/util/IO.h>
#include <openvdb/tools/LevelSetSphere.h>
#endif

#ifdef __linux__
#pragma GCC diagnostic pop
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif // VENUS_INCLUDE_GLTF

namespace venus {

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::scene::RasterContext::RenderObject)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_HERMES_FIELD(bounds)
HERMES_PUSH_DEBUG_HERMES_FIELD(transform)
HERMES_PUSH_DEBUG_FIELD(count)
HERMES_PUSH_DEBUG_FIELD(first_index)
HERMES_PUSH_DEBUG_VK_FIELD(index_buffer)
HERMES_PUSH_DEBUG_VK_FIELD(vertex_buffer)
HERMES_PUSH_DEBUG_FIELD(vertex_buffer_address)
HERMES_PUSH_DEBUG_LINE("material instance: 0x{:x}",
                       (uintptr_t)(object.material_instance.get()))
HERMES_TO_STRING_DEBUG_METHOD_END

#ifdef VENUS_INCLUDE_GLTF
HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::scene::graph::GLTF_Node)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_MAP_FIELD_BEGIN(meshes_, name, mesh_ptr)
HERMES_PUSH_DEBUG_LINE("mesh[{}]:\n\t{}\n", name, venus::to_string(*mesh_ptr))
HERMES_PUSH_DEBUG_MAP_FIELD_END
HERMES_PUSH_DEBUG_MAP_FIELD_BEGIN(mesh_storage_, name, s)
HERMES_PUSH_DEBUG_LINE("mesh_storage[{}]:\n\t{}\n\t{}\n", name,
                       venus::to_string(s.vertices),
                       venus::to_string(s.indices))
HERMES_PUSH_DEBUG_MAP_FIELD_END
HERMES_PUSH_DEBUG_MAP_FIELD_BEGIN(nodes_, name, node)
HERMES_PUSH_DEBUG_LINE("node[{}]: {:x}\n", name,
                       (node ? (uintptr_t)node.get() : 0))
HERMES_PUSH_DEBUG_MAP_FIELD_END
HERMES_PUSH_DEBUG_MAP_FIELD_BEGIN(materials_, name, material)
HERMES_PUSH_DEBUG_LINE("material[{}]: {:x}\n", name,
                       (material ? (uintptr_t)material.get() : 0))
HERMES_PUSH_DEBUG_MAP_FIELD_END
HERMES_PUSH_DEBUG_ARRAY_FIELD_BEGIN(top_nodes_, node)
HERMES_PUSH_DEBUG_LINE("top_node[{}]: {:x}\n", i,
                       (node ? (uintptr_t)node.get() : 0))
HERMES_PUSH_DEBUG_ARRAY_FIELD_END
HERMES_PUSH_DEBUG_ARRAY_FIELD_BEGIN(samplers_, sampler)
HERMES_UNUSED_VARIABLE(sampler);
HERMES_PUSH_DEBUG_VENUS_FIELD(samplers_[i])
HERMES_PUSH_DEBUG_ARRAY_FIELD_END
HERMES_PUSH_DEBUG_ARRAY_FIELD_BEGIN(image_handles_, handle)
HERMES_UNUSED_VARIABLE(handle);
HERMES_PUSH_DEBUG_VENUS_FIELD(image_handles_[i])
HERMES_PUSH_DEBUG_ARRAY_FIELD_END
HERMES_PUSH_DEBUG_VENUS_FIELD(descriptor_allocator_)
HERMES_PUSH_DEBUG_VENUS_FIELD(material_data_buffer_)
HERMES_TO_STRING_DEBUG_METHOD_END
#endif // VENUS_INCLUDE_GLTF

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::scene::graph::Node)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_LINE("{}", object.toString())
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::scene::graph::ModelNode)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_LINE("{}", object.toString())
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::scene::graph::CameraNode)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_LINE("{}", object.toString())
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::scene::graph::LabeledGraph)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_LINE("{}", object.toString())
HERMES_TO_STRING_DEBUG_METHOD_END

} // namespace venus

namespace venus::scene {

void Renderable::setVisible(bool visible) { visible_ = visible; }

} // namespace venus::scene

namespace venus::scene::graph {

Node::~Node() noexcept { this->destroy(); }

void Node::draw(const hermes::geo::Transform &top_matrix,
                DrawContext &context) {
  if (!visible_)
    return;
  auto node_matrix = top_matrix * world_matrix_;
  for (auto &child : children_)
    child->draw(node_matrix, context);
}

void Node::destroy() noexcept {
  for (auto &child : children_)
    child->destroy();
  children_.clear();
}

Node::Ptr Node::parent() { return parent_; }

void Node::setParent(Node::Ptr _parent) { parent_ = Node::Ptr::weak(_parent); }

void Node::addChild(Node::Ptr child) { children_.push_back(child); }

void Node::setLocalTransform(const hermes::geo::Transform &transform) {
  local_matrix_ = transform;
}

const hermes::geo::Transform &Node::localTransform() const {
  return local_matrix_;
}

void Node::updateTrasform(const hermes::geo::Transform &parent_matrix) {
  world_matrix_ = parent_matrix * local_matrix_;
  for (auto &child : children_)
    child->updateTrasform(world_matrix_);
}

std::string Node::toString(u32 tab_size) const {
  hermes::cstr s;
  s.appendLine(
      hermes::cstr::format("parent: 0x{:x}", (uintptr_t)parent_.get()));
  s.appendLine(
      hermes::cstr::format("local: {}", hermes::to_string(local_matrix_)));
  s.appendLine(
      hermes::cstr::format("world: {}", hermes::to_string(world_matrix_)));
  for (const auto &child : children_) {
    s.appendLine("Child: ");
    s.appendLine(hermes::cstr::format("{}", child->toString(tab_size)));
  }
  return s.str();
}

ModelNode::ModelNode(Model::Ptr model) : model_{model} {}

void ModelNode::draw(const hermes::geo::Transform &top_matrix,
                     DrawContext &context) {
  if (!visible_)
    return;
  auto model_matrix = top_matrix * world_matrix_;

  std::visit(
      DrawContextOverloaded{
          [&](RasterContext &ctx) {
            for (const auto &shape : model_->shapes()) {
              RasterContext::RenderObject render_object;
              // scene
              render_object.bounds = shape.bounds;
              render_object.transform = model_matrix;
              // mesh
              render_object.first_index = shape.index_base;
              render_object.count =
                  shape.index_count ? shape.index_count : shape.vertex_count;
              if (model_->vertexBuffer()) {
                render_object.vertex_buffer = model_->vertexBuffer();
                render_object.vertex_buffer_address =
                    model_->vertexBufferAddress();
              }
              if (model_->indexBuffer())
                render_object.index_buffer = model_->indexBuffer();
              //  shading
              render_object.material_instance = shape.material;
              ctx.objects.push_back(render_object);
            }
          },
          [&](TracerContext &ctx) {
            for (const auto &shape : model_->shapes()) {
              TracerContext::RenderObject render_object;
              // scene
              render_object.transform = model_matrix;
              render_object.transform_buffer_address =
                  model_->transformBufferAddress();
              // mesh
              render_object.vertex_layout = model_->vertexLayout();
              render_object.primitive_count =
                  (shape.index_count ? shape.index_count : shape.vertex_count) /
                  3;
              render_object.vertex_buffer_address =
                  model_->vertexBufferAddress();
              render_object.index_buffer_address = model_->indexBufferAddress();
              render_object.max_vertex = shape.vertex_count;
              // transform
              //  shading
              ctx.objects.push_back(render_object);
            }
          },
      },
      context);

  Node::draw(model_matrix, context);
}

void ModelNode::destroy() noexcept {
  model_ = Model::Ptr();
  Node::destroy();
}

Model::Ptr ModelNode::model() { return model_; }

void ModelNode::setModel(Model::Ptr model) { model_ = model; }

std::string ModelNode::toString(u32 tab_size) const {
  hermes::cstr s;
  s.appendLine(hermes::cstr::format("model: {}", venus::to_string(*model_)));
  s.appendLine(hermes::cstr::format("{}", Node::toString(tab_size)));
  return s.str();
}

CameraNode::CameraNode(Camera::Ptr camera) : camera_{camera} {}

void CameraNode::draw(const hermes::geo::Transform &top_matrix,
                      DrawContext &context) {
  if (!visible_)
    return;
  auto model_matrix = top_matrix * world_matrix_;
  Node::draw(model_matrix, context);
}

void CameraNode::destroy() noexcept {
  camera_ = Camera::Ptr();
  Node::destroy();
}

Camera::Ptr CameraNode::camera() { return camera_; }

void CameraNode::setCamera(Camera::Ptr camera) { camera_ = camera; }

std::string CameraNode::toString(u32 tab_size) const {
  hermes::cstr s;
  s.appendLine(hermes::cstr::format("camera: {}", venus::to_string(*camera_)));
  s.appendLine(hermes::cstr::format("{}", Node::toString(tab_size)));
  return s.str();
}

#ifdef VENUS_INCLUDE_GLTF
VkFilter extractFilter(fastgltf::Filter filter) {
  switch (filter) {
  // nearest samplers
  case fastgltf::Filter::Nearest:
  case fastgltf::Filter::NearestMipMapNearest:
  case fastgltf::Filter::NearestMipMapLinear:
    return VK_FILTER_NEAREST;

  // linear samplers
  case fastgltf::Filter::Linear:
  case fastgltf::Filter::LinearMipMapNearest:
  case fastgltf::Filter::LinearMipMapLinear:
  default:
    return VK_FILTER_LINEAR;
  }
}

VkSamplerMipmapMode extractMipMapMode(fastgltf::Filter filter) {
  switch (filter) {
  case fastgltf::Filter::NearestMipMapNearest:
  case fastgltf::Filter::LinearMipMapNearest:
    return VK_SAMPLER_MIPMAP_MODE_NEAREST;

  case fastgltf::Filter::NearestMipMapLinear:
  case fastgltf::Filter::LinearMipMapLinear:
  default:
    return VK_SAMPLER_MIPMAP_MODE_LINEAR;
  }
}

std::vector<Sampler> loadSamplers(const fastgltf::Asset &asset,
                                  VkDevice vk_device) {
  std::vector<Sampler> samplers;
  for (const auto &sampler : asset.samplers) {
    Sampler s;

    VENUS_ASSIGN(s,
                 Sampler::Config::defaults()
                     .setMaxLod(VK_LOD_CLAMP_NONE)
                     .setMinLod(0)
                     .setMagFilter(extractFilter(
                         sampler.magFilter.value_or(fastgltf::Filter::Nearest)))
                     .setMinFilter(extractFilter(
                         sampler.minFilter.value_or(fastgltf::Filter::Nearest)))
                     .setMipmapMode(extractMipMapMode(
                         sampler.minFilter.value_or(fastgltf::Filter::Nearest)))
                     .build(vk_device));
    samplers.emplace_back(std::move(s));
  }
  return samplers;
}

GLTF_Node::ImageData loadImage(const engine::GraphicsDevice &gd,
                               fastgltf::Image &i, fastgltf::Asset &asset) {

  GLTF_Node::ImageData image_data;

  i32 width, height, n_channels;

  auto createImageAndFreeData = [&](u8 *data) {
    if (!data)
      return;

    VkExtent3D size;
    size.width = width;
    size.height = height;
    size.depth = 1;

    auto image_r = mem::AllocatedImage::Config::forTexture(size)
                       .addUsage(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                       .build(*gd);

    if (image_r) {

      auto err = pipeline::ImageWritter()
                     .addImage(**image_r, data, size)
                     .immediateSubmit(gd);

      if (err == VeResult::noError())
        image_data.image = std::move(*image_r);
    }

    stbi_image_free(data);
  };

  std::visit(
      fastgltf::visitor{
          [](auto &arg) { HERMES_UNUSED_VARIABLE(arg); },
          [&](fastgltf::sources::URI &file_path) {
            // We don't support offsets with stbi.
            assert(file_path.fileByteOffset == 0);
            // We're only capable of loading local files.
            assert(file_path.uri.isLocalPath());

            const std::string path(file_path.uri.path().begin(),
                                   file_path.uri.path().end());
            createImageAndFreeData(
                stbi_load(path.c_str(), &width, &height, &n_channels, 4));
          },
          [&](fastgltf::sources::Vector &vector) {
            createImageAndFreeData(stbi_load_from_memory(
                reinterpret_cast<const u8 *>(vector.bytes.data()),
                static_cast<int>(vector.bytes.size()), &width, &height,
                &n_channels, 4));
          },
          [&](fastgltf::sources::BufferView &view) {
            auto &buffer_view = asset.bufferViews[view.bufferViewIndex];
            auto &buffer = asset.buffers[buffer_view.bufferIndex];

            std::visit(
                fastgltf::visitor{
                    // We only care about VectorWithMime here, because we
                    // specify LoadExternalBuffers, meaning all buffers
                    // are already loaded into a vector.
                    [](auto &arg) { HERMES_UNUSED_VARIABLE(arg); },
                    [&](fastgltf::sources::Array &vector) {
                      createImageAndFreeData(stbi_load_from_memory(
                          reinterpret_cast<const u8 *>(vector.bytes.data() +
                                                       buffer_view.byteOffset),
                          static_cast<int>(buffer_view.byteLength), &width,
                          &height, &n_channels, 4));
                    }},
                buffer.data);
          },
      },
      i.data);

  if (image_data.image) {
    auto view_r =
        mem::Image::View::Config()
            .setViewType(VK_IMAGE_VIEW_TYPE_2D)
            .setFormat(image_data.image.format())
            .setSubresourceRange({VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1})
            .build(image_data.image);
    if (view_r) {
      image_data.view = std::move(*view_r);
    }
  }

  return image_data;
}

materials::GLTF_MetallicRoughness::Data loadMaterialData(
    fastgltf::Material &material,
    const materials::GLTF_MetallicRoughness::Resources &resources) {
  auto &cache = engine::GraphicsEngine::cache();

  materials::GLTF_MetallicRoughness::Data constants;
  constants.color_factors.x = material.pbrData.baseColorFactor[0];
  constants.color_factors.y = material.pbrData.baseColorFactor[1];
  constants.color_factors.z = material.pbrData.baseColorFactor[2];
  constants.color_factors.w = material.pbrData.baseColorFactor[3];
  constants.metal_rough_factors.x = material.pbrData.metallicFactor;
  constants.metal_rough_factors.y = material.pbrData.roughnessFactor;
  constants.color_tex_id =
      cache.textures().add(resources.color_image.view, resources.color_sampler);
  constants.metal_rough_tex_id = cache.textures().add(
      resources.metal_rough_image.view, resources.metal_rough_sampler);
  return constants;
}

materials::GLTF_MetallicRoughness::Resources
loadMaterialResources(fastgltf::Material &material, fastgltf::Asset &asset,
                      u32 material_index, VkBuffer material_data_buffer,
                      const std::vector<Sampler> &samplers,
                      const std::vector<mem::Image::Handle> &images) {
  materials::GLTF_MetallicRoughness::Resources resources;
  // default the material textures
  resources.color_image =
      engine::GraphicsEngine::globals().defaults.error_image;
  resources.color_sampler =
      engine::GraphicsEngine::globals().defaults.linear_sampler;
  resources.metal_rough_image =
      engine::GraphicsEngine::globals().defaults.error_image;
  resources.metal_rough_sampler =
      engine::GraphicsEngine::globals().defaults.linear_sampler;
  // set the uniform buffer for the material data
  resources.data_buffer = material_data_buffer;
  resources.data_buffer_offset =
      material_index * sizeof(materials::GLTF_MetallicRoughness::Data);

  // grab textures from gltf file
  if (material.pbrData.baseColorTexture.has_value()) {
    size_t img_idx =
        asset.textures[material.pbrData.baseColorTexture.value().textureIndex]
            .imageIndex.value();
    size_t sampler_idx =
        asset.textures[material.pbrData.baseColorTexture.value().textureIndex]
            .samplerIndex.value();
    resources.color_image = images[img_idx];
    resources.color_sampler = *samplers[sampler_idx];
  }
  return resources;
}

VeResult
loadMeshes(fastgltf::Asset &asset, const engine::GraphicsDevice &gd,
           const std::vector<Material::Instance::Ptr> &materials,
           std::unordered_map<std::string, Model::Ptr> &meshes,
           std::unordered_map<std::string, Model::Storage<mem::AllocatedBuffer>>
               &mesh_storage,
           std::vector<Model::Ptr> &flatten_meshes) {
  struct Vertex {
    hermes::geo::point3 position;
    f32 uv_x;
    hermes::geo::vec3 normal;
    f32 uv_y;
    hermes::colors::RGBA_Color color;
  };

  std::vector<u32> indices;
  std::vector<Vertex> vertices;

  for (fastgltf::Mesh &mesh : asset.meshes) {
    indices.clear();
    vertices.clear();

    Model::Config model_config;

    for (auto &&p : mesh.primitives) {
      Model::Shape surface;
      surface.index_base = (u32)indices.size();
      surface.index_count =
          (u32)asset.accessors[p.indicesAccessor.value()].count;

      size_t initial_vtx = vertices.size();

      // load indexes
      {
        fastgltf::Accessor &index_accessor =
            asset.accessors[p.indicesAccessor.value()];
        indices.reserve(indices.size() + index_accessor.count);

        fastgltf::iterateAccessor<std::uint32_t>(
            asset, index_accessor,
            [&](std::uint32_t idx) { indices.push_back(idx + initial_vtx); });
      }

      // load vertex positions
      {
        fastgltf::Accessor &position_accessor =
            asset.accessors[p.findAttribute("POSITION")->accessorIndex];
        vertices.resize(vertices.size() + position_accessor.count);

        fastgltf::iterateAccessorWithIndex<glm::vec3>(
            asset, position_accessor, [&](glm::vec3 v, size_t index) {
              Vertex vtx;
              vtx.position.x = v.x;
              vtx.position.y = v.y;
              vtx.position.z = v.z;
              vtx.normal = {1, 0, 0};
              vtx.color = {1, 1, 1, 1};
              vtx.uv_x = 0;
              vtx.uv_y = 0;
              vertices[initial_vtx + index] = vtx;
            });
      }

      // load vertex normals
      auto normals = p.findAttribute("NORMAL");
      if (normals != p.attributes.end()) {
        fastgltf::iterateAccessorWithIndex<glm::vec3>(
            asset, asset.accessors[(*normals).accessorIndex],
            [&](glm::vec3 v, size_t index) {
              vertices[initial_vtx + index].normal.x = v.x;
              vertices[initial_vtx + index].normal.y = v.y;
              vertices[initial_vtx + index].normal.z = v.z;
            });
      }

      // load UVs
      auto uv = p.findAttribute("TEXCOORD_0");
      if (uv != p.attributes.end()) {
        fastgltf::iterateAccessorWithIndex<glm::vec2>(
            asset, asset.accessors[(*uv).accessorIndex],
            [&](glm::vec2 v, size_t index) {
              vertices[initial_vtx + index].uv_x = v.x;
              vertices[initial_vtx + index].uv_y = v.y;
            });
      }

      // load vertex colors
      auto colors = p.findAttribute("COLOR_0");
      if (colors != p.attributes.end()) {

        fastgltf::iterateAccessorWithIndex<glm::vec4>(
            asset, asset.accessors[(*colors).accessorIndex],
            [&](glm::vec4 v, size_t index) {
              vertices[initial_vtx + index].color.r = v.x;
              vertices[initial_vtx + index].color.g = v.y;
              vertices[initial_vtx + index].color.b = v.z;
              vertices[initial_vtx + index].color.a = v.w;
            });
      }

      if (p.materialIndex.has_value()) {
        surface.material = materials[p.materialIndex.value()];
      } else {
        surface.material = materials[0];
      }

      hermes::geo::point3 minpos = vertices[initial_vtx].position;
      hermes::geo::point3 maxpos = vertices[initial_vtx].position;
      for (u32 i = initial_vtx; i < vertices.size(); i++)
        for (u32 j = 0; j < 3; ++j) {
          minpos[j] = std::min(minpos[j], vertices[i].position[j]);
          maxpos[j] = std::max(maxpos[j], vertices[i].position[j]);
        }
      surface.bounds.setCenter((maxpos + hermes::geo::vec3(minpos)) / 2.f);
      surface.bounds.setRadius(((maxpos - minpos) / 2.f).length());

      model_config.addShape(surface);
    }

    Model::Storage<mem::AllocatedBuffer> storage;

    VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
        storage.vertices,
        mem::AllocatedBuffer::Config::forStorage(
            sizeof(Vertex) * vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
            .build(*gd));

    VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
        storage.indices,
        mem::AllocatedBuffer::Config::forStorage(
            sizeof(u32) * indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
            .build(*gd));

    // copy data

    VENUS_RETURN_BAD_RESULT(pipeline::BufferWritter()
                                .addBuffer(*storage.vertices, vertices.data(),
                                           sizeof(Vertex) * vertices.size())
                                .addBuffer(*storage.indices, indices.data(),
                                           sizeof(u32) * indices.size())
                                .immediateSubmit(gd));

    Model model;

    VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
        model,
        model_config
            .setVertices(*storage.vertices, storage.vertices.deviceAddress())
            .setIndices(*storage.indices, storage.indices.deviceAddress())
            .build());

    auto mesh_key = mesh.name.c_str();

    meshes[mesh_key] = Model::Ptr::shared();
    *meshes[mesh_key] = std::move(model);

    mesh_storage[mesh.name.c_str()] = std::move(storage);

    flatten_meshes.emplace_back(meshes[mesh_key]);
  }
  return VeResult::noError();
}

Result<GLTF_Node::Ptr> GLTF_Node::from(const std::filesystem::path &path,
                                       const engine::GraphicsDevice &gd) {
  if (!std::filesystem::exists(path)) {
#ifdef __linux__
    HERMES_ERROR("File does not exist: {}", path.c_str());
#endif
    return VeResult::ioError();
  } else {
#ifdef __linux__
    HERMES_INFO("Loading GLTF: {}", path.c_str());
#endif
  }

  // Creates a Parser instance. Optimally, you should reuse this across loads,
  // but don't use it across threads.
  static constexpr auto supported_extensions =
      fastgltf::Extensions::KHR_mesh_quantization |
      fastgltf::Extensions::KHR_texture_transform |
      fastgltf::Extensions::KHR_materials_variants;
  fastgltf::Parser parser(supported_extensions);

  auto gltf_file = fastgltf::MappedGltfFile::FromPath(path);
  if (!bool(gltf_file)) {
    HERMES_ERROR("Failed to open glTF file: {}",
                 fastgltf::getErrorMessage(gltf_file.error()));
    return VeResult::ioError();
  }

  constexpr auto gltf_options = fastgltf::Options::DontRequireValidAssetMember |
                                fastgltf::Options::AllowDouble |
                                fastgltf::Options::LoadExternalBuffers |
                                fastgltf::Options::LoadExternalImages |
                                fastgltf::Options::GenerateMeshIndices;
  // This loads the glTF file into the gltf object and parses the JSON.
  // It automatically detects whether this is a JSON-based or binary glTF.
  // If you know the type, you can also use loadGltfJson or loadGltfBinary.
  auto asset =
      parser.loadGltf(gltf_file.get(), path.parent_path(), gltf_options);
  if (auto error = asset.error(); error != fastgltf::Error::None) {
    HERMES_ERROR("Some error occurred while reading the buffer, parsing the "
                 "JSON, or validating the data.");
    HERMES_ERROR("Error: {} - {}", fastgltf::getErrorName(error),
                 fastgltf::getErrorMessage(error));
    return VeResult::ioError();
  }

  // Optionally, you can now also call the
  // fastgltf::validate method. This will
  // more strictly enforce the glTF spec and is not needed
  // most of the time, though I would certainly recommend it
  // in a development environment or when debugging to avoid
  // mishaps.
#ifdef VENUS_DEBUG
  if (auto error = fastgltf::validate(asset.get());
      error != fastgltf::Error::None) {
    HERMES_ERROR("Error on validating asset: {}",
                 fastgltf::getErrorName(error));
    return VeResult::ioError();
  }
#endif

  GLTF_Node::Ptr scene = GLTF_Node::Ptr::shared();
  /////////////////////////////////////////////////////////////////////////////
  // SAMPLERS
  /////////////////////////////////////////////////////////////////////////////
  scene->samplers_ = loadSamplers(asset.get(), **gd);

  /////////////////////////////////////////////////////////////////////////////
  // IMAGES
  /////////////////////////////////////////////////////////////////////////////

  for (auto &gltf_image : asset->images) {
    auto data = loadImage(gd, gltf_image, asset.get());
    if ((bool)data.image && (bool)data.view) {
      mem::Image::Handle handle;
      handle.image = *data.image;
      handle.view = *data.view;
      scene->images_[gltf_image.name.c_str()] = std::move(data);
      scene->image_handles_.push_back(handle);
    } else {
      scene->image_handles_.push_back(
          engine::GraphicsEngine::globals().defaults.error_image);
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  // UNIFORM BUFFERS
  // one for each material, but since we have only one type of material we
  // replicate it as many materials we read
  /////////////////////////////////////////////////////////////////////////////

  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      scene->material_data_buffer_,
      mem::AllocatedBuffer::Config::forUniform(
          sizeof(materials::GLTF_MetallicRoughness::Data) *
          asset->materials.size())
          .build(*gd));

  // we can estimate the descriptors we will need accurately
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      scene->descriptor_allocator_,
      pipeline::DescriptorAllocator::Config()
          .setInitialSetCount(asset->materials.size())
          .addDescriptorType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3.f)
          .addDescriptorType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3.f)
          .addDescriptorType(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1.f)
          .build(**gd));

  /////////////////////////////////////////////////////////////////////////////
  // MATERIALS
  /////////////////////////////////////////////////////////////////////////////
  std::vector<Material::Instance::Ptr> materials(asset->materials.size());

  VENUS_DECLARE_OR_RETURN_BAD_RESULT(mem::DeviceMemory::ScopedMap, d,
                                     scene->material_data_buffer_.scopedMap());

  materials::GLTF_MetallicRoughness::Data *data =
      d.get<materials::GLTF_MetallicRoughness::Data>();
#if __cplusplus >= 202302L
  for (const auto &[material_index, gltf_material] :
       std::views::enumerate(asset->materials)) {
#else
  for (u32 material_index = 0; material_index < asset->materials.size();
       ++material_index) {
    const auto &gltf_material = asset->materials[material_index];
#endif
    // GLTF_Material::Ptr new_material = GLTF_Material::Ptr();
    // scene->materials_[material.name.c_str()] = new_material;

    // TODO: transparency support
    // MaterialPass passType = MaterialPass::MainColor;
    // if (mat.alphaMode == fastgltf::AlphaMode::Blend) {
    //   passType = MaterialPass::Transparent;
    // }
    materials::GLTF_MetallicRoughness parameters;
    // resources
    parameters.resources = loadMaterialResources(
        gltf_material, asset.get(), material_index,
        *scene->material_data_buffer_, scene->samplers_, scene->image_handles_);
    // constants (stored in the uniform buffer)
    data[material_index] = parameters.data =
        loadMaterialData(gltf_material, parameters.resources);

    // write material
    VENUS_DECLARE_SHARED_PTR_FROM_RESULT_OR_RETURN_BAD_RESULT(
        Material::Instance, m_instance,
        parameters.write(scene->descriptor_allocator_,
                         &engine::GraphicsEngine::globals()
                              .materials.gltf_metallic_roughness));

    materials[material_index] = scene->materials_[gltf_material.name.c_str()] =
        m_instance;
  }

  /////////////////////////////////////////////////////////////////////////////
  // MESHES
  /////////////////////////////////////////////////////////////////////////////

  std::vector<Model::Ptr> meshes;
  VENUS_CHECK_VE_RESULT(loadMeshes(asset.get(), gd, materials, scene->meshes_,
                                   scene->mesh_storage_, meshes));

  /////////////////////////////////////////////////////////////////////////////
  // NODES
  /////////////////////////////////////////////////////////////////////////////

  std::vector<Node::Ptr> nodes;
  for (fastgltf::Node &node : asset->nodes) {
    auto new_node = Node::Ptr::shared();

    // find if the node has a mesh, and if it does hook it to the mesh pointer
    // and allocate it with the meshnode class
    if (node.meshIndex.has_value()) {
      new_node = ModelNode::Ptr::shared();
      static_cast<ModelNode *>(new_node.get())
          ->setModel(meshes[*node.meshIndex]);
    }

    nodes.push_back(new_node);
    scene->nodes_[node.name.c_str()];
    std::visit(fastgltf::visitor{
                   [&](fastgltf::math::fmat4x4 matrix) {
                     // glm::mat4x4 m(matrix.data());
                     //  memcpy(&m, matrix.data(), sizeof(matrix));
                     // new_node->setLocalTransform(m);
                     HERMES_UNUSED_VARIABLE(matrix);
                   },
                   [&](fastgltf::TRS transform) {
                     hermes::geo::vec3 tl(transform.translation[0],
                                          transform.translation[1],
                                          transform.translation[2]);
                     hermes::geo::quat rot(
                         transform.rotation[3], transform.rotation[0],
                         transform.rotation[1], transform.rotation[2]);
                     hermes::geo::vec3 sc(transform.scale[0],
                                          transform.scale[1],
                                          transform.scale[2]);

                     auto tm = hermes::geo::Transform::translate(tl);
                     auto rm = hermes::geo::Transform(rot.matrix());
                     auto sm = hermes::geo::Transform::scale(sc.x, sc.y, sc.z);
                     new_node->setLocalTransform(tm * rm * sm);
                   }},
               node.transform);
  }

  /////////////////////////////////////////////////////////////////////////////
  // GRAPH
  /////////////////////////////////////////////////////////////////////////////
  // run loop again to setup transform hierarchy
  for (u32 i = 0; i < asset->nodes.size(); i++) {
    fastgltf::Node &node = asset->nodes[i];
    Node::Ptr &scene_node = nodes[i];

    for (auto &c : node.children) {
      scene_node->addChild(nodes[c]);
      nodes[c]->setParent(scene_node);
    }
  }

  // find the top nodes, with no parents
  for (auto &node : nodes) {
    if (!node->parent()) {
      scene->top_nodes_.push_back(node);
      node->updateTrasform(hermes::geo::Transform());
    }
  }

  return Result<GLTF_Node::Ptr>(std::move(scene));
}

GLTF_Node::~GLTF_Node() noexcept {
  destroy();
  Node::destroy();
}

void GLTF_Node::destroy() noexcept {
  nodes_.clear();
  top_nodes_.clear();
  meshes_.clear();
  mesh_storage_.clear();
  materials_.clear();
  descriptor_allocator_.destroy();
  material_data_buffer_.destroy();
  image_handles_.clear();
  for (auto &item : images_) {
    item.second.view.destroy();
    item.second.image.destroy();
  }
  images_.clear();
  samplers_.clear();
}

void GLTF_Node::draw(const hermes::geo::Transform &top_matrix,
                     DrawContext &ctx) {
  if (!visible_)
    return;
  for (auto &n : top_nodes_) {
    n->draw(top_matrix, ctx);
  }
}

std::string GLTF_Node::toString(u32 tab_size) const {
  hermes::cstr s;
  s.appendLine(hermes::cstr::format("gltf node"));
  s.appendLine(hermes::cstr::format("{}", Node::toString(tab_size)));
  return s.str();
}
#endif // VENUS_INCLUDE_GLTF

#ifdef VENUS_INCLUDE_VDB

Result<VDB_Node::Ptr> VDB_Node::from(const std::filesystem::path &vdb_file_path,
                                     const engine::GraphicsDevice &gd) {
  auto vdb_node = VDB_Node::Ptr::shared();

  HERMES_UNUSED_VARIABLE(vdb_file_path);
  try {

    openvdb::initialize();

    // Create a VDB file object.
    openvdb::io::File file("smoke2.vdb");
    file.open();

    openvdb::GridBase::Ptr baseGrid;
    for (openvdb::io::File::NameIterator nameIter = file.beginName();
         nameIter != file.endName(); ++nameIter) {
      // Read in only the grid we are interested in.
      if (nameIter.gridName() == "density") {
        baseGrid = file.readGrid(nameIter.gridName());
      } else {
        std::cout << "skipping grid " << nameIter.gridName() << std::endl;
      }
    }
    file.close();

    openvdb::FloatGrid::Ptr grid =
        openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);
    // Convert the OpenVDB grid, srcGrid, into a NanoVDB grid handle.
    auto handle = nanovdb::tools::createNanoGrid(*grid);
    // Define a (raw) pointer to the NanoVDB grid on the host. Note we match the
    // value type of the srcGrid!
    auto *dstGrid = handle.grid<float>();
    handle.size();
    if (!dstGrid)
      throw std::runtime_error(
          "GridHandle does not contain a grid with value type float");

    hermes::geo::bounds::bbox3 box;
    box.lower.x = dstGrid->worldBBox().min()[0];
    box.lower.y = dstGrid->worldBBox().min()[1];
    box.lower.z = dstGrid->worldBBox().min()[2];

    box.upper.x = dstGrid->worldBBox().max()[0];
    box.upper.y = dstGrid->worldBBox().max()[1];
    box.upper.z = dstGrid->worldBBox().max()[2];

    VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
        vdb_node->bounds_model_,
        AllocatedModel::Config::fromShape(shapes::box, box,
                                          shape_option_bits::vertices)
            .build(gd));

    // send grid to gpu

    VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
        vdb_node->gpu_vdb_data_,
        mem::AllocatedBuffer::Config::forStorage(
            handle.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
            .build(*gd));

    // uniform buffer

    VENUS_RETURN_BAD_RESULT(
        pipeline::BufferWritter()
            .addBuffer(*(vdb_node->gpu_vdb_data_), handle.data(), handle.size())
            .immediateSubmit(gd));

    VENUS_ASSIGN_OR_RETURN_BAD_RESULT(vdb_node->material_data_buffer_,
                                      mem::AllocatedBuffer::Config::forUniform(
                                          sizeof(materials::VDB_Volume::Data))
                                          .build(*gd));

    VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
        vdb_node->descriptor_allocator_,
        pipeline::DescriptorAllocator::Config()
            .setInitialSetCount(1)
            .addDescriptorType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1.f)
            .build(**gd));

    // copy data

    materials::VDB_Volume parameters;
    parameters.data.vdb_buffer = vdb_node->gpu_vdb_data_.deviceAddress();
    parameters.resources.data_buffer = *(vdb_node->material_data_buffer_);
    parameters.resources.data_buffer_offset = 0;

    {
      VENUS_DECLARE_OR_RETURN_BAD_RESULT(
          mem::DeviceMemory::ScopedMap, d,
          vdb_node->material_data_buffer_.scopedMap());
      materials::VDB_Volume::Data *data = d.get<materials::VDB_Volume::Data>();
      *data = parameters.data;
    }

    VENUS_DECLARE_SHARED_PTR_FROM_RESULT_OR_RETURN_BAD_RESULT(
        Material::Instance, m_instance,
        parameters.write(vdb_node->descriptor_allocator_,
                         &engine::GraphicsEngine::globals().materials.vdb));

    vdb_node->bounds_model_.setMaterial(0, m_instance);

  } catch (const std::exception &e) {
    HERMES_ERROR("An exception occurred: \"{}\"", e.what());
    return VeResult::extError();
  }

  return Result<VDB_Node::Ptr>(vdb_node);
}

VDB_Node::~VDB_Node() noexcept { destroy(); }

void VDB_Node::draw(const hermes::geo::Transform &top_matrix,
                    DrawContext &draw_ctx) {
  if (!visible_)
    return;
  auto model_matrix = top_matrix * world_matrix_;

  std::visit(DrawContextOverloaded{
                 [&](RasterContext &ctx) {
                   for (const auto &shape : bounds_model_.shapes()) {
                     RasterContext::RenderObject render_object;
                     // scene
                     render_object.bounds = shape.bounds;
                     render_object.transform = model_matrix;
                     // mesh
                     render_object.first_index = shape.index_base;
                     render_object.count = shape.index_count
                                               ? shape.index_count
                                               : shape.vertex_count;
                     if (bounds_model_.vertexBuffer()) {
                       render_object.vertex_buffer =
                           bounds_model_.vertexBuffer();
                       render_object.vertex_buffer_address =
                           bounds_model_.vertexBufferAddress();
                     }
                     if (bounds_model_.indexBuffer()) {
                       render_object.index_buffer = bounds_model_.indexBuffer();
                     }
                     //  shading
                     render_object.material_instance = shape.material;
                     ctx.objects.push_back(render_object);
                   }
                 },
                 [&](TracerContext &ctx) { HERMES_UNUSED_VARIABLE(ctx); }},
             draw_ctx);
  Node::draw(model_matrix, draw_ctx);
}

void VDB_Node::destroy() noexcept {
  material_data_buffer_.destroy();
  descriptor_allocator_.destroy();
  gpu_vdb_data_.destroy();
  bounds_model_.destroy();

  Node::destroy();
}

std::string VDB_Node::toString(u32 tab_size) const {
  hermes::cstr s;
  s.appendLine(hermes::cstr::format("vdb node"));
  s.appendLine(hermes::cstr::format("{}", Node::toString(tab_size)));
  return s.str();
}

#endif // VENUS_INCLUDE_VDB

LabeledGraph::~LabeledGraph() noexcept { destroy(); }

void LabeledGraph::destroy() noexcept {
  for (auto &node : nodes_)
    node.second->destroy();
  nodes_.clear();
}

LabeledGraph &LabeledGraph::add(const std::string &name, const Node::Ptr &node,
                                const std::string &parent) {
  nodes_.insert(std::make_pair(name, node));
  if (parent.empty())
    addChild(node);
  else {
    HERMES_ASSERT(nodes_.contains(parent));
    nodes_[parent]->addChild(node);
  }
  return *this;
}

LabeledGraph &LabeledGraph::addModel(const std::string &name,
                                     const Model::Ptr &model,
                                     const std::string &parent) {
  return add(name, ModelNode::Ptr::shared(model), parent);
}

LabeledGraph &LabeledGraph::addCamera(const std::string &name,
                                      const Camera::Ptr &camera,
                                      const std::string &parent) {
  return add(name, CameraNode::Ptr::shared(camera), parent);
}

void LabeledGraph::draw(const hermes::geo::Transform &top_matrix,
                        DrawContext &context) {
  if (!visible_)
    return;
  Node::draw(top_matrix, context);
}

std::string LabeledGraph::toString(u32 tab_size) const {
  hermes::cstr s;
  for (const auto &item : nodes_) {
    s.appendLine(hermes::cstr::format("node {} -> 0x{:x}", item.first,
                                      (uintptr_t)item.second.get()));
  }
  s.appendLine(hermes::cstr::format("{}", Node::toString(tab_size)));
  return s.str();
}

} // namespace venus::scene::graph
