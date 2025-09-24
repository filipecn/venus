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

/// \file   gltf_io.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30
/// \brief  GLTF support.

#pragma once

#include <venus/engine/graphics_device.h>
#include <venus/mem/image.h>
#include <venus/pipeline/shader_module.h>
#include <venus/scene/material.h>
#include <venus/scene/model.h>
#include <venus/scene/scene_graph.h>
#include <venus/scene/texture.h>

#include <hermes/geometry/vector.h>

namespace venus::io {

class GLTF_File {};

} // namespace venus::io

namespace venus::scene {

class GLTF_MetallicRoughness : public Material::Writer {
public:
  static Result<Material> material(const engine::GraphicsDevice &gd);

  struct Data {
    hermes::geo::vec4 color_factors;
    hermes::geo::vec4 metal_rough_factors;
    hermes::geo::vec4 extra[14];
  };

  static_assert(sizeof(Data) == 256);

  struct Resources {
    mem::Image::Handle color_image;
    VkSampler color_sampler;
    mem::Image::Handle metal_rough_image;
    VkSampler metal_rough_sampler;
    VkBuffer data_buffer;
    u32 data_buffer_offset;
  };

  Result<Material::Instance>
  write(pipeline::DescriptorAllocator &allocator) override;

  Data data;
  Resources resources;

  VENUS_TO_STRING_FRIEND(GLTF_MetallicRoughness);
};

class GLTF_Node : public Node {
public:
  using Ptr = std::shared_ptr<GLTF_Node>;

  static Result<Ptr> from(const std::filesystem::path &path,
                          const engine::GraphicsDevice &gd);

  ~GLTF_Node() noexcept;

  void draw(const hermes::geo::Transform &top_matrix,
            DrawContext &ctx) override;

  void destroy() noexcept;

private:
  // named data

  // std::unordered_map<std::string, Image> images_;
  std::unordered_map<std::string, Model::Ptr> meshes_;
  std::unordered_map<std::string, Model::Storage<mem::AllocatedBuffer>>
      mesh_storage_;
  std::unordered_map<std::string, Node::Ptr> nodes_;
  std::unordered_map<std::string, Material::Instance::Ptr> materials_;

  // top nodes on the GLTF tree
  std::vector<Node::Ptr> top_nodes_;

  // constructed data

  std::vector<Sampler> samplers_;
  std::vector<mem::Image::Handle> image_handles_;

  pipeline::DescriptorAllocator descriptor_allocator_;

  mem::AllocatedBuffer material_data_buffer_;
};

} // namespace venus::scene
