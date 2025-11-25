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

/// \file   swapchain.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30

#include <venus/io/swapchain.h>

#include <venus/utils/vk_debug.h>

namespace venus::io {

Swapchain::Config &Swapchain::Config::setSurface(VkSurfaceKHR surface) {
  surface_ = surface;
  return *this;
}

Swapchain::Config &
Swapchain::Config::setOldSwapchain(VkSwapchainKHR old_swapchain) {
  old_swapchain_ = old_swapchain;
  return *this;
}

Swapchain::Config &Swapchain::Config::addUsageFlags(VkImageUsageFlags flags) {
  usage_flags_ |= flags;
  return *this;
}

Swapchain::Config &Swapchain::Config::setQueueFamilyIndices(
    const core::vk::GraphicsQueueFamilyIndices &family_indices) {
  family_indices_ = family_indices;
  return *this;
}

Swapchain::Config &Swapchain::Config::setExtent(const VkExtent2D &extent) {
  extent_ = extent;
  return *this;
}

Swapchain::Config &Swapchain::Config::setPresentMode(VkPresentModeKHR mode) {
  present_mode_ = mode;
  return *this;
}

Swapchain::Config &Swapchain::Config::setFormat(VkFormat format) {
  surface_format_.format = format;
  return *this;
}

Swapchain::Config &
Swapchain::Config::setColorSpace(VkColorSpaceKHR color_space) {
  surface_format_.colorSpace = color_space;
  return *this;
}

Swapchain::Config &Swapchain::Config::setImageCount(u32 image_count) {
  image_count_ = image_count;
  return *this;
}

Swapchain::Config &
Swapchain::Config::addCreateFlags(VkSwapchainCreateFlagsKHR flags) {
  flags_ |= flags;
  return *this;
}

Result<Swapchain> Swapchain::Config::build(const core::Device &device) const {

  // copy config values that might change
  auto extent = extent_;

  // ***************************************************************************
  // 1. Get available presentation modes
  // ***************************************************************************
  VkPresentModeKHR present_mode;
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      present_mode,
      device.physical().selectPresentationMode(surface_, present_mode_));

  // ***************************************************************************
  // 2. Get available surface formats
  // ***************************************************************************
  VkSurfaceFormatKHR surface_format;
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      surface_format, device.physical().selectFormatOfSwapchainImages(
                          surface_, surface_format_));

  // ***************************************************************************
  // 3. Get available swapchain image count
  // ***************************************************************************
  VkSurfaceCapabilitiesKHR surface_capabilities;
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      surface_capabilities, device.physical().surfaceCapabilities(surface_));

  if (surface_capabilities.currentExtent.width ==
      (std::numeric_limits<uint32_t>::max)()) {
    // surface size is undefined
    extent.width =
        std::clamp(extent.width, surface_capabilities.minImageExtent.width,
                   surface_capabilities.maxImageExtent.width);
    extent.height =
        std::clamp(extent.height, surface_capabilities.minImageExtent.height,
                   surface_capabilities.maxImageExtent.height);
  } else {
    // If the surface size is defined, the swap chain size must match
    extent = surface_capabilities.currentExtent;
  }

  // ***************************************************************************
  // 4. Get available surface transform
  // ***************************************************************************
  VkSurfaceTransformFlagBitsKHR pre_transform =
      (surface_capabilities.supportedTransforms &
       VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
          ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
          : surface_capabilities.currentTransform;

  // ***************************************************************************
  // 5. Get available composite alpha
  // ***************************************************************************
  VkCompositeAlphaFlagBitsKHR composite_alpha =
      (surface_capabilities.supportedCompositeAlpha &
       VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR)
          ? VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR
      : (surface_capabilities.supportedCompositeAlpha &
         VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR)
          ? VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR
      : (surface_capabilities.supportedCompositeAlpha &
         VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR)
          ? VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
          : VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

  // ***************************************************************************
  // 6. Configure swapchain
  // ***************************************************************************

  VkSwapchainCreateInfoKHR create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  create_info.pNext = nullptr;
  create_info.flags = flags_;
  create_info.surface = surface_;
  create_info.minImageCount =
      std::clamp(image_count_, surface_capabilities.minImageCount,
                 surface_capabilities.maxImageCount);
  create_info.imageFormat = surface_format.format;
  create_info.imageColorSpace = surface_format.colorSpace;
  create_info.imageExtent = extent;
  create_info.imageArrayLayers = 1;
  create_info.imageUsage = usage_flags_;
  create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  create_info.queueFamilyIndexCount = 0;
  create_info.pQueueFamilyIndices = nullptr;
  create_info.preTransform = pre_transform;
  create_info.compositeAlpha = composite_alpha;
  create_info.presentMode = present_mode;
  create_info.clipped = VK_TRUE;
  create_info.oldSwapchain = old_swapchain_;

  if (family_indices_.graphics_queue_family_index !=
      family_indices_.present_queue_family_index) {
    u32 queue_family_indices[2] = {family_indices_.graphics_queue_family_index,
                                   family_indices_.present_queue_family_index};
    // If the graphics and present queues are from different queue families, we
    // either have to explicitly transfer ownership of images between the
    // queues, or we have to create the swapchain with CONCURRENT sharing mode.
    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    create_info.queueFamilyIndexCount = 2;
    create_info.pQueueFamilyIndices = queue_family_indices;
  }

  Swapchain swapchain;

  VENUS_VK_RETURN_BAD_RESULT(vkCreateSwapchainKHR(
      *device, &create_info, nullptr, &swapchain.vk_swapchain_));

  std::vector<VkImage> images;
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      images, core::vk::acquireSwapchainImages(*device, *swapchain));

  for (auto vk_image : images) {
    mem::Image image;
    VENUS_ASSIGN_OR_RETURN_BAD_RESULT(image,
                                      mem::Image::Config()
                                          .setFormat(surface_format.format)
                                          .build(*device, vk_image));
    mem::Image::View view;
    VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
        view, mem::Image::View::Config()
                  .setViewType(VK_IMAGE_VIEW_TYPE_2D)
                  .setFormat(surface_format.format)
                  .setSubresourceRange({VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1})
                  .build(image));

    swapchain.images_.emplace_back(std::move(image));
    swapchain.image_views_.emplace_back(std::move(view));
  }

  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      swapchain.depth_buffer_,
      mem::AllocatedImage::Config::forDepthBuffer(extent).build(device));

  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      swapchain.depth_buffer_view_,
      mem::Image::View::Config()
          .setViewType(VK_IMAGE_VIEW_TYPE_2D)
          .setFormat(swapchain.depth_buffer_.format())
          .setSubresourceRange({VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1})
          .build(swapchain.depth_buffer_));

  swapchain.color_format_ = surface_format.format;
  swapchain.extent_ = extent;
  swapchain.vk_device_ = *device;
#ifdef VENUS_DEBUG
  swapchain.config_ = *this;
#endif
  return Result<Swapchain>(std::move(swapchain));
}

Swapchain::Swapchain(Swapchain &&rhs) noexcept { *this = std::move(rhs); }

Swapchain::~Swapchain() noexcept { destroy(); }

Swapchain &Swapchain::operator=(Swapchain &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void Swapchain::swap(Swapchain &rhs) noexcept {
  VENUS_SWAP_FIELD_WITH_RHS(vk_swapchain_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_device_);
  VENUS_SWAP_FIELD_WITH_RHS(images_);
  VENUS_SWAP_FIELD_WITH_RHS(image_views_);
  VENUS_FIELD_SWAP_RHS(depth_buffer_);
  VENUS_FIELD_SWAP_RHS(depth_buffer_view_);
  VENUS_SWAP_FIELD_WITH_RHS(color_format_);
  VENUS_SWAP_FIELD_WITH_RHS(extent_);
#ifdef VENUS_DEBUG
  VENUS_SWAP_FIELD_WITH_RHS(config_);
#endif
}

void Swapchain::destroy() noexcept {
  image_views_.clear();
  images_.clear();
  depth_buffer_view_.destroy();
  depth_buffer_.destroy();
  if (vk_device_ && vk_swapchain_) {
    vkDestroySwapchainKHR(vk_device_, vk_swapchain_, nullptr);
    vk_swapchain_ = VK_NULL_HANDLE;
    vk_device_ = VK_NULL_HANDLE;
  }
}

VkSwapchainKHR Swapchain::operator*() const { return vk_swapchain_; }

VkExtent2D Swapchain::imageExtent() const { return extent_; }

u32 Swapchain::imageCount() const { return images_.size(); }

VkFormat Swapchain::colorFormat() const { return color_format_; }

const mem::Image &Swapchain::depthBuffer() const { return depth_buffer_; }

const std::vector<mem::Image> &Swapchain::images() const { return images_; }

const mem::Image::View &Swapchain::depthBufferView() const {
  return depth_buffer_view_;
}

const std::vector<mem::Image::View> &Swapchain::imageViews() const {
  return image_views_;
}

Result<u32> Swapchain::nextImage(VkSemaphore vk_semaphore, VkFence vk_fence) {

  u32 image_index = 0;
  VkResult e = vkAcquireNextImageKHR(vk_device_, vk_swapchain_, 1000000000,
                                     vk_semaphore, vk_fence, &image_index);
  if (e == VK_ERROR_OUT_OF_DATE_KHR) {
    // when a swapchain is not valid/adequate anymore we need to recreate the
    // swapchain with new parameters. For that, we need to destroy the old
    // swapchain and the objects related to the swapchain, to recreate them
    // later
    HERMES_WARN("swapchain image out of date.");
    // resize_requested = true;
    return VeResult::noError();
  }

  if (e != VK_SUCCESS && e != VK_SUBOPTIMAL_KHR) {
    HERMES_ERROR("error on getting next swapchain image!");
    return VeResult::error();
  }

  if (image_index >= images_.size())
    return VeResult::error();

  return Result<u32>(image_index);
}

} // namespace venus::io

namespace venus {
HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::io::Swapchain::Config)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_VK_FIELD(surface_);
HERMES_PUSH_DEBUG_VK_STRING(VkImageUsageFlags, usage_flags_);
HERMES_PUSH_DEBUG_LINE("extent: {}x{}", object.extent_.width,
                       object.extent_.height);
HERMES_PUSH_DEBUG_VK_STRING(VkPresentModeKHR, present_mode_);
HERMES_PUSH_DEBUG_VK_STRING(VkFormat, surface_format_.format);
HERMES_PUSH_DEBUG_VK_STRING(VkColorSpaceKHR, surface_format_.colorSpace);
HERMES_PUSH_DEBUG_LINE("queues: {}", venus::to_string(object.family_indices_));
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::io::Swapchain)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_VENUS_FIELD(config_);

HERMES_TO_STRING_DEBUG_METHOD_END

} // namespace venus
