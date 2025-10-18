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

/// \file   texture.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30
/// \brief  Scene texture.

#pragma once

#include <venus/core/device.h>
#include <venus/mem/image.h>

namespace venus::scene {

struct Sampler {
  struct Config {
    static Config defaults();

    Config();
    Config &setFlags(VkSamplerCreateFlags flags);
    Config &setMagFilter(VkFilter filter);
    Config &setMinFilter(VkFilter filter);
    Config &setMipmapMode(VkSamplerMipmapMode mode);
    Config &setAddressModeU(VkSamplerAddressMode mode);
    Config &setAddressModeV(VkSamplerAddressMode mode);
    Config &setAddressModeW(VkSamplerAddressMode mode);
    Config &setMipLodBias(f32 bias);
    Config &setAnisotropyEnable(bool enable);
    Config &setMaxAnisotropy(f32 anisotropy);
    Config &setCompareEnable(bool enable);
    Config &setCompareOp(VkCompareOp op);
    Config &setMinLod(f32 lod);
    Config &setMaxLod(f32 lod);
    Config &setBorderColor(VkBorderColor color);

    Result<Sampler> create(VkDevice vk_device) const;

  private:
    VkSamplerCreateInfo info_;
  };

  VENUS_DECLARE_RAII_FUNCTIONS(Sampler)

  void destroy() noexcept;
  void swap(Sampler &rhs);
  VkSampler operator*() const;

private:
  VkSampler vk_sampler_{VK_NULL_HANDLE};
  VkDevice vk_device_{VK_NULL_HANDLE};

  VENUS_to_string_FRIEND(Sampler);
};

struct Texture {
  VkImageView image;
  Sampler sampler;
  VkImageLayout image_layout;

  VENUS_to_string_FRIEND(Texture);
};

/// The texture cache holds an array of textures that can be accessed by
/// bindless shaders or arbitrarily. Cached textures can own their data, be
/// self allocated or just image handles.
class TextureCache {
public:
  u32 add(const VkImageView &image, VkSampler sampler);
  void clear();

  u32 size() const;
  const std::vector<VkDescriptorImageInfo> &operator*() const;

private:
  std::vector<VkDescriptorImageInfo> cache_;

  VENUS_to_string_FRIEND(TextureCache);
};

} // namespace venus::scene
