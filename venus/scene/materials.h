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

/// \file   materials.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30
/// \brief  Built-in venus materials.

#include <hermes/numeric/matrix.h>
#include <venus/engine/graphics_device.h>
#include <venus/mem/image.h>
#include <venus/pipeline/shader_module.h>
#include <venus/scene/material.h>

#include <hermes/geometry/transform.h>
#include <hermes/geometry/vector.h>

namespace venus::scene::materials {

class Material_Test : public Material::Writer {
public:
  static Result<Material> material(const engine::GraphicsDevice &gd);

  struct Data {
    hermes::math::mat4 projection;
    hermes::math::mat4 view;
  };

  struct Resources {
    VkBuffer data_buffer;
    u32 data_buffer_offset;
  };

  Result<Material::Instance> write(pipeline::DescriptorAllocator &allocator,
                                   const Material *material) override;

  Data data;
  Resources resources;
};

class Material_BindlessTest : public Material_Test {
public:
  static Result<Material> material(const engine::GraphicsDevice &gd);

  struct PushConstants {
    VkDeviceAddress vertex_buffer;
  };
};

#ifdef VENUS_INCLUDE_GLTF
class GLTF_MetallicRoughness : public Material::Writer {
public:
  static Result<Material> material(const engine::GraphicsDevice &gd);

  struct Data {
    hermes::geo::vec4 color_factors;
    hermes::geo::vec4 metal_rough_factors;
    u32 color_tex_id;
    u32 metal_rough_tex_id;
    u32 pad[2];
    hermes::geo::vec4 extra[13];
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

  Result<Material::Instance> write(pipeline::DescriptorAllocator &allocator,
                                   const Material *material) override;

  Data data;
  Resources resources;

  VENUS_to_string_FRIEND(GLTF_MetallicRoughness);
};
#endif // VENUS_INCLUDE_GLTF

#ifdef VENUS_INCLUDE_VDB
class VDB_Volume : public Material::Writer {
public:
  static Result<Material> material(const engine::GraphicsDevice &gd);

  struct Data {
    VkDeviceAddress vdb_buffer;
    hermes::geo::point3 camera_pos;
    u32 pad[3];
    hermes::geo::vec4 extra[14];
  };

  static_assert(sizeof(Data) == 256);

  struct Resources {
    VkBuffer data_buffer;
    u32 data_buffer_offset;
  };

  Result<Material::Instance> write(pipeline::DescriptorAllocator &allocator,
                                   const Material *material) override;

  Data data;
  Resources resources;

  VENUS_to_string_FRIEND(VDB_Volume);
};
#endif // VENUS_INCLUDE_VDB

} // namespace venus::scene::materials
