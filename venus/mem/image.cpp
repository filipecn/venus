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

namespace venus {
HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::mem::Image::View::Config)
HERMES_PUSH_DEBUG_VK_STRING(VkImageViewCreateFlags, info_.flags)
HERMES_PUSH_DEBUG_VK_FIELD(info_.image);
HERMES_PUSH_DEBUG_VK_STRING(VkImageViewType, info_.viewType)
HERMES_PUSH_DEBUG_VK_STRING(VkFormat, info_.format);
HERMES_PUSH_DEBUG_VK_STRING(VkComponentSwizzle, info_.components.r)
HERMES_PUSH_DEBUG_VK_STRING(VkComponentSwizzle, info_.components.g)
HERMES_PUSH_DEBUG_VK_STRING(VkComponentSwizzle, info_.components.b)
HERMES_PUSH_DEBUG_VK_STRING(VkComponentSwizzle, info_.components.a)
HERMES_PUSH_DEBUG_VK_STRING(VkImageAspectFlags,
                            info_.subresourceRange.aspectMask)
HERMES_PUSH_DEBUG_FIELD(info_.subresourceRange.baseMipLevel)
HERMES_PUSH_DEBUG_FIELD(info_.subresourceRange.levelCount)
HERMES_PUSH_DEBUG_FIELD(info_.subresourceRange.baseArrayLayer)
HERMES_PUSH_DEBUG_FIELD(info_.subresourceRange.layerCount)
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::mem::Image::View)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_VK_FIELD(vk_image_view_);
HERMES_PUSH_DEBUG_VK_FIELD(vk_device_);
HERMES_PUSH_DEBUG_VENUS_FIELD(config_);
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::mem::Image::Handle)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_VK_FIELD(image);
HERMES_PUSH_DEBUG_VK_FIELD(view);
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::mem::Image::Config)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_VK_STRING(VkFormatFeatureFlags, format_features_)
HERMES_PUSH_DEBUG_VK_STRING(VkImageAspectFlags, aspect_mask_)
HERMES_PUSH_DEBUG_VK_STRING(VkImageCreateFlags, info_.flags)
HERMES_PUSH_DEBUG_VK_STRING(VkImageType, info_.imageType)
HERMES_PUSH_DEBUG_VK_STRING(VkFormat, info_.format)
HERMES_PUSH_DEBUG_LINE("extent = {}x{}x{}\n", object.info_.extent.width,
                       object.info_.extent.height, object.info_.extent.depth)
HERMES_PUSH_DEBUG_FIELD(info_.mipLevels)
HERMES_PUSH_DEBUG_FIELD(info_.arrayLayers)
HERMES_PUSH_DEBUG_VK_STRING(VkSampleCountFlagBits, info_.samples)
HERMES_PUSH_DEBUG_VK_STRING(VkImageTiling, info_.tiling)
HERMES_PUSH_DEBUG_VK_STRING(VkImageUsageFlags, info_.usage)
HERMES_PUSH_DEBUG_VK_STRING(VkSharingMode, info_.sharingMode)
HERMES_PUSH_DEBUG_FIELD(info_.queueFamilyIndexCount)
HERMES_PUSH_DEBUG_VK_STRING(VkImageLayout, info_.initialLayout)
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::mem::Image)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_VK_FIELD(vk_image_);
HERMES_PUSH_DEBUG_VK_FIELD(vk_device_);
HERMES_PUSH_DEBUG_VK_STRING(VkFormat, vk_format_);
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::mem::AllocatedImage)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_VK_FIELD(vk_image_);
HERMES_PUSH_DEBUG_VK_FIELD(vk_device_);
HERMES_PUSH_DEBUG_VK_STRING(VkFormat, vk_format_);
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::mem::ImagePool)
HERMES_PUSH_DEBUG_TITLE
HERMES_TO_STRING_DEBUG_METHOD_END

} // namespace venus

namespace venus::mem {

template <>
Result<Image>
Image::Setup<Image::Config, Image>::build(VkDevice vk_device,
                                          VkImage vk_image) const {
  Image image;
  image.vk_device_ = vk_device;
  image.vk_image_ = vk_image;
  image.vk_format_ = info_.format;
  image.ownership_ = false;
  return Result<Image>(std::move(image));
}

template <>
Result<Image>
Image::Setup<Image::Config, Image>::build(const core::Device &device) const {

  Image image;
  auto info = createInfo();

  VENUS_VK_RETURN_BAD_RESULT(
      vkCreateImage(*device, &info, nullptr, &image.vk_image_));

  image.vk_device_ = *device;
  image.vk_format_ = info.format;
#ifdef VENUS_DEBUG
  image.config_ = static_cast<const Image::Config &>(*this);
#endif

  return Result<Image>(std::move(image));
}

VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Image::View, setFlags,
                                          VkImageViewCreateFlags, flags)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Image::View, setImage, VkImage, image)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Image::View, setViewType,
                                          VkImageViewType, viewType)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Image::View, setFormat, VkFormat,
                                          format)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Image::View, setComponents,
                                          VkComponentMapping, components)
VENUS_DEFINE_SET_CONFIG_INFO_FIELD_METHOD(Image::View, setSubresourceRange,
                                          VkImageSubresourceRange,
                                          subresourceRange)

Result<Image::View> Image::View::Config::build(const Image &image) const {
  auto info = info_;
  info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  info.pNext = nullptr;
  info.image = *image;

  Image::View view;
  view.vk_device_ = image.device();
  VENUS_VK_RETURN_BAD_RESULT(
      vkCreateImageView(view.vk_device_, &info, nullptr, &view.vk_image_view_));
#ifdef VENUS_DEBUG
  view.config_ = *this;
#endif
  return Result<Image::View>(std::move(view));
}

Image::View::View(Image::View &&rhs) noexcept { *this = std::move(rhs); }

Image::View::~View() noexcept { destroy(); }

Image::View &Image::View::operator=(Image::View &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void Image::View::swap(Image::View &rhs) noexcept {
  VENUS_SWAP_FIELD_WITH_RHS(vk_device_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_image_view_);
#ifdef VENUS_DEBUG
  VENUS_SWAP_FIELD_WITH_RHS(config_);
#endif
}

void Image::View::destroy() noexcept {
  if (vk_device_ && vk_image_view_)
    vkDestroyImageView(vk_device_, vk_image_view_, nullptr);
  vk_device_ = VK_NULL_HANDLE;
  vk_image_view_ = VK_NULL_HANDLE;
}

VkImageView Image::View::operator*() const { return vk_image_view_; }

Image::View::operator bool() const { return vk_image_view_ != VK_NULL_HANDLE; }

Image::Image(Image &&rhs) noexcept { *this = std::move(rhs); }

Image::~Image() noexcept { destroy(); }

Image &Image::operator=(Image &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void Image::swap(Image &rhs) noexcept {
  VENUS_SWAP_FIELD_WITH_RHS(vk_image_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_device_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_format_);
  VENUS_SWAP_FIELD_WITH_RHS(ownership_);
#ifdef VENUS_DEBUG
  VENUS_SWAP_FIELD_WITH_RHS(config_);
#endif
}

void Image::destroy() noexcept {
  if (vk_device_ && vk_image_ && ownership_) {
    vkDestroyImage(vk_device_, vk_image_, nullptr);
  }
  vk_device_ = VK_NULL_HANDLE;
  vk_image_ = VK_NULL_HANDLE;
}

VkImage Image::operator*() const { return vk_image_; }

Image::operator bool() const { return vk_image_ != VK_NULL_HANDLE; }

VkFormat Image::format() const { return vk_format_; }

VkDevice Image::device() const { return vk_device_; }

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(AllocatedImage, setImageConfig,
                                     const Image::Config &,
                                     image_config_ = value)

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(AllocatedImage, setMemoryConfig,
                                     const DeviceMemory::Config &,
                                     mem_config_ = value)

Result<AllocatedImage>
AllocatedImage::Config::build(const core::Device &device) const {
  auto info = image_config_.createInfo();

  auto alloc_info = vma_allocation_create_info_; // mem_config_.allocationInfo();
  VkImage vk_image{VK_NULL_HANDLE};
  VmaAllocation vma_allocation{VK_NULL_HANDLE};
  VENUS_VK_RETURN_BAD_RESULT(vmaCreateImage(device.allocator(), &info,
                                            &alloc_info, &vk_image,
                                            &vma_allocation, nullptr));
  AllocatedImage image;
  image.vma_allocator_ = device.allocator();
  image.vma_allocation_ = vma_allocation;
  image.vk_format_ = info.format;
  image.vk_image_ = vk_image;
  image.vk_device_ = *device;

  return Result<AllocatedImage>(std::move(image));
}

AllocatedImage::AllocatedImage(AllocatedImage &&rhs) noexcept {
  *this = std::move(rhs);
}

AllocatedImage::~AllocatedImage() noexcept { destroy(); }

AllocatedImage &AllocatedImage::operator=(AllocatedImage &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void AllocatedImage::swap(AllocatedImage &rhs) noexcept {
  VENUS_SWAP_FIELD_WITH_RHS(vk_image_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_device_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_format_);
  VENUS_SWAP_FIELD_WITH_RHS(vma_allocator_);
  VENUS_SWAP_FIELD_WITH_RHS(vma_allocation_);
}

void AllocatedImage::destroy() noexcept {
  if (vk_image_ && vma_allocator_ && vma_allocation_) {
    vmaDestroyImage(vma_allocator_, vk_image_, vma_allocation_);
  }
  vma_allocation_ = VK_NULL_HANDLE;
  vk_device_ = VK_NULL_HANDLE;
  vk_image_ = VK_NULL_HANDLE;
}

ImagePool::ImagePool(ImagePool &&rhs) noexcept { *this = std::move(rhs); }

ImagePool::~ImagePool() noexcept { destroy(); }

ImagePool &ImagePool::operator=(ImagePool &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void ImagePool::destroy() noexcept {
  for (auto &item : images_) {
    item.second.view.destroy();
    item.second.image.destroy();
  }
  images_.clear();
}

void ImagePool::swap(ImagePool &rhs) { VENUS_SWAP_FIELD_WITH_RHS(images_); }

VeResult ImagePool::addImageView(const std::string &image_name,
                                 const Image::View::Config &image_view_config) {
  auto it = images_.find(image_name);
  if (it == images_.end()) {
    HERMES_ERROR("An image view can only be added to an ImagePool for an "
                 "existent image in the pool. Image with name <{}> not found.",
                 image_name);
    return VeResult::notFound();
  }
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(it->second.view,
                                    image_view_config.build(it->second.image));
  return VeResult::noError();
}

Result<VkImage> ImagePool::operator[](const std::string &name) const {
  auto it = images_.find(name);
  if (it == images_.end())
    return VeResult::notFound();
  return *(it->second.image);
}

} // namespace venus::mem
