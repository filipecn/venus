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

/// \file   gltf_io.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30

#include <venus/engine/gltf_io.h>

#include <venus/engine/graphics_engine.h>
#include <venus/utils/vk_debug.h>

#include <hermes/geometry/quaternion.h>

#ifdef __linux__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wredundant-move"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/util.hpp>
#ifdef __linux__
#pragma GCC diagnostic pop
#endif

namespace venus {

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::scene::GLTF_MetallicRoughness)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_HERMES_FIELD(data.color_factors)
HERMES_PUSH_DEBUG_HERMES_FIELD(data.metal_rough_factors)
HERMES_PUSH_DEBUG_VENUS_FIELD(resources.color_image)
HERMES_PUSH_DEBUG_VK_FIELD(resources.color_sampler)
HERMES_PUSH_DEBUG_VENUS_FIELD(resources.metal_rough_image)
HERMES_PUSH_DEBUG_VK_FIELD(resources.metal_rough_sampler)
HERMES_PUSH_DEBUG_VK_FIELD(resources.data_buffer)
HERMES_PUSH_DEBUG_FIELD(resources.data_buffer_offset)
HERMES_TO_STRING_DEBUG_METHOD_END

} // namespace venus

namespace venus::scene {

Result<Material>
GLTF_MetallicRoughness::material(const engine::GraphicsDevice &gd) {

  // descriptor set layouts

  pipeline::DescriptorSet::Layout l;
  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      l, pipeline::DescriptorSet::Layout::Config()
             .addLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
                               VK_SHADER_STAGE_VERTEX_BIT |
                                   VK_SHADER_STAGE_FRAGMENT_BIT)
             .create(**gd));
  auto vk_descriptor_set_layout = *l;

  auto &globals = engine::GraphicsEngine::globals();

  Material m;

  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      m,
      Material::Config()
          .setDescriptorSetLayout(std::move(l))
          .setMaterialPipelineConfig(
              Material::Pipeline::Config()
                  .setPipelineConfig(
                      pipeline::GraphicsPipeline::Config::defaults(
                          gd.swapchain().imageExtent())
                          .setColorAttachmentFormat(
                              gd.swapchain().colorFormat())
                          .setDepthFormat(gd.swapchain().depthBuffer().format())
                          .addShaderStage(
                              pipeline::Pipeline::ShaderStage()
                                  .setStages(VK_SHADER_STAGE_VERTEX_BIT)
                                  .create(globals.shaders.vert_mesh))
                          .addShaderStage(
                              pipeline::Pipeline::ShaderStage()
                                  .setStages(VK_SHADER_STAGE_FRAGMENT_BIT)
                                  .create(globals.shaders.frag_mesh_pbr)))
                  .setPipelineLayoutConfig(
                      pipeline::Pipeline::Layout::Config()
                          .addDescriptorSetLayout(
                              globals.descriptors.scene_data_layout)
                          .addDescriptorSetLayout(vk_descriptor_set_layout)
                          .addPushConstantRange(
                              VK_SHADER_STAGE_VERTEX_BIT, 0,
                              sizeof(engine::GraphicsEngine::Globals::Types::
                                         DrawPushConstants))))
          .create(**gd, *gd.renderpass()));

  return Result<Material>(std::move(m));
}

Result<Material::Instance>
GLTF_MetallicRoughness::write(pipeline::DescriptorAllocator &allocator) {
  auto &globals = engine::GraphicsEngine::globals();

  Material::Instance instance;

  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      instance.descriptor_set,
      allocator.allocate(
          *globals.materials.gltf_metallic_roughness.descriptorSetLayout()));
  descriptor_writer_.clear();
  descriptor_writer_.writeBuffer(
      0, resources.data_buffer, sizeof(GLTF_MetallicRoughness::Data),
      resources.data_buffer_offset, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
  descriptor_writer_.update(instance.descriptor_set);
  instance.material = &globals.materials.gltf_metallic_roughness;

  return Result<Material::Instance>(std::move(instance));
}

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

    VENUS_ASSIGN_RESULT(
        s, Sampler::Config::defaults()
               .setMaxLod(VK_LOD_CLAMP_NONE)
               .setMinLod(0)
               .setMagFilter(extractFilter(
                   sampler.magFilter.value_or(fastgltf::Filter::Nearest)))
               .setMinFilter(extractFilter(
                   sampler.minFilter.value_or(fastgltf::Filter::Nearest)))
               .setMipmapMode(extractMipMapMode(
                   sampler.minFilter.value_or(fastgltf::Filter::Nearest)))
               .create(vk_device));
    samplers.emplace_back(std::move(s));
  }
  return samplers;
}

mem::Image loadImage(VkDevice vk_device, fastgltf::Image &i) {
  HERMES_UNUSED_VARIABLE(vk_device);
  HERMES_UNUSED_VARIABLE(i);
  return {};
}

GLTF_MetallicRoughness::Data
loadMaterialConstants(fastgltf::Material &material) {
  GLTF_MetallicRoughness::Data constants;
  constants.color_factors.x = material.pbrData.baseColorFactor[0];
  constants.color_factors.y = material.pbrData.baseColorFactor[1];
  constants.color_factors.z = material.pbrData.baseColorFactor[2];
  constants.color_factors.w = material.pbrData.baseColorFactor[3];
  constants.metal_rough_factors.x = material.pbrData.metallicFactor;
  constants.metal_rough_factors.y = material.pbrData.roughnessFactor;
  // TODO
  // constants.colorTexID = engine->texCache
  //                           .AddTexture(materialResources.colorImage.imageView,
  //                                       materialResources.colorSampler)
  //                           .Index;
  // constants.metalRoughTexID =
  //    engine->texCache
  //        .AddTexture(materialResources.metalRoughImage.imageView,
  //                    materialResources.metalRoughSampler)
  //        .Index;

  return constants;
}

GLTF_MetallicRoughness::Resources
loadMaterialResources(fastgltf::Material &material, fastgltf::Asset &asset,
                      u32 material_index, VkBuffer material_data_buffer,
                      const std::vector<Sampler> &samplers,
                      const std::vector<mem::Image::Handle> &images) {
  GLTF_MetallicRoughness::Resources resources;
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
      material_index * sizeof(GLTF_MetallicRoughness::Data);

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

    VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
        storage.vertices,
        mem::AllocatedBuffer::Config()
            .setBufferConfig(mem::Buffer::Config::forStorage(sizeof(Vertex) *
                                                             vertices.size())
                                 .enableShaderDeviceAddress())
            .setMemoryConfig(mem::DeviceMemory::Config().setDeviceLocal())
            .create(*gd));

    VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
        storage.indices,
        mem::AllocatedBuffer::Config()
            .setBufferConfig(
                mem::Buffer::Config::forStorage(sizeof(u32) * indices.size())
                    .enableShaderDeviceAddress())
            .setMemoryConfig(mem::DeviceMemory::Config().setDeviceLocal())
            .create(*gd));

    // copy data

    VENUS_RETURN_BAD_RESULT(pipeline::BufferWritter()
                                .addBuffer(*storage.vertices, vertices.data(),
                                           sizeof(Vertex) * vertices.size())
                                .addBuffer(*storage.indices, indices.data(),
                                           sizeof(u32) * indices.size())
                                .immediateSubmit(gd));

    Model model;

    VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
        model,
        model_config
            .setVertices(*storage.vertices, storage.vertices.deviceAddress())
            .setIndices(*storage.indices)
            .create());

    auto mesh_key = mesh.name.c_str();

    meshes[mesh_key] = std::make_shared<Model>();
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

  GLTF_Node::Ptr scene = std::make_shared<GLTF_Node>();
  /////////////////////////////////////////////////////////////////////////////
  // SAMPLERS
  /////////////////////////////////////////////////////////////////////////////
  scene->samplers_ = loadSamplers(asset.get(), **gd);

  /////////////////////////////////////////////////////////////////////////////
  // IMAGES
  /////////////////////////////////////////////////////////////////////////////

  for (auto &image : asset->images) {
    HERMES_UNUSED_VARIABLE(image);
    scene->image_handles_.push_back(
        engine::GraphicsEngine::globals().defaults.error_image);
  }

  /////////////////////////////////////////////////////////////////////////////
  // UNIFORM BUFFERS
  // one for each material, but since we have only one type of material we
  // replicate it as many materials we read
  /////////////////////////////////////////////////////////////////////////////

  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      scene->material_data_buffer_,
      mem::AllocatedBuffer::Config()
          .setBufferConfig(mem::Buffer::Config::forUniform(
              sizeof(GLTF_MetallicRoughness::Data) * asset->materials.size()))
          .setMemoryConfig(
              mem::DeviceMemory::Config().setHostVisible().setUsage(
                  VMA_MEMORY_USAGE_CPU_TO_GPU))
          .create(*gd));

  // we can estimate the descriptors we will need accurately
  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      scene->descriptor_allocator_,
      pipeline::DescriptorAllocator::Config()
          .setInitialSetCount(asset->materials.size())
          .addDescriptorType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3.f)
          .addDescriptorType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3.f)
          .addDescriptorType(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1.f)
          .create(**gd));

  /////////////////////////////////////////////////////////////////////////////
  // MATERIALS
  /////////////////////////////////////////////////////////////////////////////
  std::vector<Material::Instance::Ptr> materials(asset->materials.size());

  VENUS_CHECK_VE_RESULT(scene->material_data_buffer_.access([&](void *d) {
    GLTF_MetallicRoughness::Data *data =
        reinterpret_cast<GLTF_MetallicRoughness::Data *>(d);
#if __cplusplus >= 202302L
    for (const auto &[material_index, gltf_material] :
         std::views::enumerate(asset->materials)) {
#else
    for (u32 index = 0; index < asset->materials.size(); ++index) {
      const auto &material = asset->materials[index];
#endif
      // GLTF_Material::Ptr new_material = GLTF_Material::Ptr();
      // scene->materials_[material.name.c_str()] = new_material;

      // TODO: transparency support
      // MaterialPass passType = MaterialPass::MainColor;
      // if (mat.alphaMode == fastgltf::AlphaMode::Blend) {
      //   passType = MaterialPass::Transparent;
      // }
      GLTF_MetallicRoughness parameters;
      // constants (stored in the uniform buffer)
      data[material_index] = parameters.data =
          loadMaterialConstants(gltf_material);
      // resources
      parameters.resources =
          loadMaterialResources(gltf_material, asset.get(), material_index,
                                *scene->material_data_buffer_, scene->samplers_,
                                scene->image_handles_);
      // write material

      Material::Instance m_instance;

      VENUS_ASSIGN_RESULT_OR_RETURN_VOID(
          m_instance, parameters.write(scene->descriptor_allocator_));

      materials[material_index] =
          scene->materials_[gltf_material.name.c_str()] =
              std::make_shared<Material::Instance>();

      *materials[material_index] = std::move(m_instance);
    }
  }));

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
    std::shared_ptr<Node> new_node;

    // find if the node has a mesh, and if it does hook it to the mesh pointer
    // and allocate it with the meshnode class
    if (node.meshIndex.has_value()) {
      new_node = std::make_shared<ModelNode>();
      static_cast<ModelNode *>(new_node.get())
          ->setModel(meshes[*node.meshIndex]);
    } else {
      new_node = std::make_shared<Node>();
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
    if (node->parent().lock() == nullptr) {
      scene->top_nodes_.push_back(node);
      node->updateTrasform(hermes::geo::Transform());
    }
  }

  return Result<GLTF_Node::Ptr>(std::move(scene));
}

GLTF_Node::~GLTF_Node() noexcept { destroy(); }

void GLTF_Node::destroy() noexcept {
  nodes_.clear();
  top_nodes_.clear();
  meshes_.clear();
  mesh_storage_.clear();
  materials_.clear();
  descriptor_allocator_.destroy();
  material_data_buffer_.destroy();
  image_handles_.clear();
  samplers_.clear();
}

void GLTF_Node::draw(const hermes::geo::Transform &top_matrix,
                     DrawContext &ctx) {
  for (auto &n : top_nodes_) {
    n->draw(top_matrix, ctx);
  }
}

} // namespace venus::scene
