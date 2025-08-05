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

/// \file   image.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30

#include <venus/mem/image.h>

#include <venus/utils/vk_debug.h>

namespace venus::mem {

Image::Config Image::Config::defaults(const VkExtent2D &extent,
                                      VkFormat format) {
  return Image::Config()
      .addCreateFlags({})
      .setImageType(VK_IMAGE_TYPE_2D)
      .setFormat(format)
      .setExtent(VkExtent3D(extent.width, extent.height, 1))
      .setMipLevels(1)
      .setArrayLayers(1)
      .setSamples(VK_SAMPLE_COUNT_1_BIT)
      .setTiling(VK_IMAGE_TILING_LINEAR)
      .addUsage(VK_IMAGE_USAGE_SAMPLED_BIT)
      .setSharingMode(VK_SHARING_MODE_EXCLUSIVE)
      .setInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
      .addAspectMask(VK_IMAGE_ASPECT_COLOR_BIT);
}

Image::Config Image::Config::forDepthBuffer(const VkExtent2D &extent,
                                            VkFormat format) {
  return Image::Config()
      .addCreateFlags({})
      .setImageType(VK_IMAGE_TYPE_2D)
      .setFormat(format)
      .setExtent(VkExtent3D(extent.width, extent.height, 1))
      .setMipLevels(1)
      .setArrayLayers(1)
      .setSamples(VK_SAMPLE_COUNT_1_BIT)
      .setTiling(VK_IMAGE_TILING_OPTIMAL)
      .addUsage(VK_IMAGE_USAGE_SAMPLED_BIT |
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
      .setSharingMode(VK_SHARING_MODE_EXCLUSIVE)
      .setInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
      .addAspectMask(VK_IMAGE_ASPECT_DEPTH_BIT);
}

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(Image::Config, setInfo, VkImageCreateInfo,
                                     info_ = value)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Image::Config, addCreateFlags,
                                          VkImageCreateFlags, flags)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Image::Config, setImageType,
                                          VkImageType, imageType)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Image::Config, setFormat, VkFormat,
                                          format)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Image::Config, setExtent, VkExtent3D,
                                          extent)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Image::Config, setMipLevels, u32,
                                          mipLevels)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Image::Config, setArrayLayers, u32,
                                          arrayLayers)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Image::Config, setSamples,
                                          VkSampleCountFlagBits, samples)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Image::Config, setTiling,
                                          VkImageTiling, tiling)
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(Image::Config, addUsage, VkImageUsageFlags,
                                     info_.usage |= value)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Image::Config, setSharingMode,
                                          VkSharingMode, sharingMode)
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(Image::Config, addQueueFamilyIndex, u32,
                                     queue_family_indices_.emplace_back(value))
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Image::Config, setInitialLayout,
                                          VkImageLayout, initialLayout)
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(Image::Config, addAspectMask,
                                     VkImageAspectFlags, aspect_mask_ |= value)
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(Image::Config, addFormatFeatures,
                                     VkFormatFeatureFlagBits,
                                     format_features_ |= value)

Result<Image> Image::Config::create(VkDevice vk_device) const {
  auto info = info_;
  info.pQueueFamilyIndices = queue_family_indices_.data();
  info.queueFamilyIndexCount = queue_family_indices_.size();

  Image image;
  image.vk_device_ = vk_device;
  image.vk_format_ = info.format;
#ifdef VENUS_DEBUG
  image.config_ = *this;
#endif

  VENUS_VK_RETURN_BAD_RESULT(
      vkCreateImage(vk_device, &info, nullptr, &image.vk_image_));
  return Result<Image>(std::move(image));
}

VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Image::View::Config, setFlags,
                                          VkImageViewCreateFlags, flags)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Image::View::Config, setImage,
                                          VkImage, image)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Image::View::Config, setViewType,
                                          VkImageViewType, viewType)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Image::View::Config, setFormat,
                                          VkFormat, format)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Image::View::Config, setComponents,
                                          VkComponentMapping, components)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Image::View::Config,
                                          setSubresourceRange,
                                          VkImageSubresourceRange,
                                          subresourceRange)

Result<Image::View> Image::View::Config::create(const Image &image) const {
  auto info = info_;
  info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  info.pNext = nullptr;
  info.image = *image;

  Image::View view;
  view.vk_device_ = image.vk_device_;
  VENUS_VK_RETURN_BAD_RESULT(vkCreateImageView(image.vk_device_, &info_,
                                               nullptr, &view.vk_image_view_));
  return Result<Image::View>(std::move(view));
}

Image::View::View(Image::View &&rhs) noexcept { *this = std::move(rhs); }

Image::View::~View() noexcept { destroy(); }

Image::View &Image::View::operator=(Image::View &&rhs) noexcept {
  destroy();
  core::vk::swap(vk_device_, rhs.vk_device_);
  core::vk::swap(vk_image_view_, rhs.vk_image_view_);
  return *this;
}

void Image::View::destroy() noexcept {
  if (vk_device_ && vk_image_view_)
    vkDestroyImageView(vk_device_, vk_image_view_, nullptr);
  vk_device_ = VK_NULL_HANDLE;
  vk_image_view_ = VK_NULL_HANDLE;
}

VkImageView Image::View::operator*() const { return vk_image_view_; }

Image::Image(Image &&rhs) noexcept { *this = std::move(rhs); }

Image::~Image() noexcept { destroy(); }

Image &Image::operator=(Image &&rhs) noexcept {
  destroy();
  core::vk::swap(vk_device_, rhs.vk_device_);
  core::vk::swap(vk_image_, rhs.vk_image_);
  vk_format_ = rhs.vk_format_;
#ifdef VENUS_DEBUG
  config_ = std::move(rhs.config_);
#endif
  return *this;
}

void Image::destroy() noexcept {
  if (vk_device_ && vk_image_)
    vkDestroyImage(vk_device_, vk_image_, nullptr);
  vk_device_ = VK_NULL_HANDLE;
  vk_image_ = VK_NULL_HANDLE;
}

VkImage Image::operator*() const { return vk_image_; }

VkFormat Image::format() const { return vk_format_; }

} // namespace venus::mem

namespace venus {
HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::mem::Image::Config)
HERMES_PUSH_DEBUG_TITLE
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::mem::Image)
HERMES_PUSH_DEBUG_TITLE
HERMES_TO_STRING_DEBUG_METHOD_END

} // namespace venus
