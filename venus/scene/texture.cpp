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

/// \file   texture.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30

#include <venus/scene/texture.h>

#include <venus/utils/vk_debug.h>

namespace venus {

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::scene::Sampler)
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::scene::Texture)
HERMES_TO_STRING_DEBUG_METHOD_END

} // namespace venus

namespace venus::scene {

VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Sampler, setFlags,
                                          VkSamplerCreateFlags, flags)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Sampler, setMagFilter, VkFilter,
                                          magFilter)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Sampler, setMinFilter, VkFilter,
                                          minFilter)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Sampler, setMipmapMode,
                                          VkSamplerMipmapMode, mipmapMode)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Sampler, setAddressModeU,
                                          VkSamplerAddressMode, addressModeU)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Sampler, setAddressModeV,
                                          VkSamplerAddressMode, addressModeV)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Sampler, setAddressModeW,
                                          VkSamplerAddressMode, addressModeW)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Sampler, setMipLodBias, f32,
                                          mipLodBias)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Sampler, setAnisotropyEnable, bool,
                                          anisotropyEnable)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Sampler, setMaxAnisotropy, f32,
                                          maxAnisotropy)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Sampler, setCompareEnable, bool,
                                          compareEnable)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Sampler, setCompareOp, VkCompareOp,
                                          compareOp)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Sampler, setMinLod, f32, minLod)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Sampler, setMaxLod, f32, maxLod)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Sampler, setBorderColor,
                                          VkBorderColor, borderColor)

Sampler::Config Sampler::Config::defaults() {
  Sampler::Config config;
  config.info_.flags = {};
  config.info_.magFilter = VK_FILTER_LINEAR;
  config.info_.minFilter = VK_FILTER_LINEAR;
  config.info_.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  config.info_.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  config.info_.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  config.info_.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  config.info_.mipLodBias = 0.0f;
  config.info_.anisotropyEnable = false;
  config.info_.maxAnisotropy = 16.0f;
  config.info_.compareEnable = false;
  config.info_.compareOp = VK_COMPARE_OP_NEVER;
  config.info_.minLod = 0.0f;
  config.info_.maxLod = 0.0f;
  config.info_.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
  return config;
}

Result<Sampler> Sampler::Config::create(VkDevice vk_device) const {
  Sampler sampler;
  VENUS_VK_RETURN_BAD_RESULT(
      vkCreateSampler(vk_device, &info_, nullptr, &sampler.vk_sampler_));
  sampler.vk_device_ = vk_device;
  return Result<Sampler>(std::move(sampler));
}

Sampler::Sampler(Sampler &&rhs) noexcept { *this = std::move(rhs); }

Sampler::~Sampler() noexcept { destroy(); }

Sampler &Sampler::operator=(Sampler &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void Sampler::destroy() noexcept {
  if (vk_device_ && vk_sampler_)
    vkDestroySampler(vk_device_, vk_sampler_, nullptr);
  vk_device_ = VK_NULL_HANDLE;
  vk_sampler_ = VK_NULL_HANDLE;
}

void Sampler::swap(Sampler &rhs) {
  VENUS_SWAP_FIELD_WITH_RHS(vk_device_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_sampler_);
}

VkSampler Sampler::operator*() const { return vk_sampler_; }

} // namespace venus::scene
