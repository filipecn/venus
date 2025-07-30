/* Copyright (c) 2019, FilipeCN.
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

/// \file   vulkan_physical_device.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-06-07

#include "venus/core/debug.h"
#include <venus/core/physical_device.h>

#include <venus/core/vk_debug.h>

namespace venus::core {

PhysicalDevice::PhysicalDevice(VkPhysicalDevice handle) {
  if (!setHandle(handle))
    return;
}

PhysicalDevice::PhysicalDevice(PhysicalDevice &&rhs) noexcept {
  *this = std::move(rhs);
  rhs.clear();
}

PhysicalDevice::PhysicalDevice(const PhysicalDevice &rhs) { *this = rhs; }

PhysicalDevice &PhysicalDevice::operator=(const PhysicalDevice &rhs) {
  vk_device_ = rhs.vk_device_;
  vk_extensions_ = rhs.vk_extensions_;
  vk_features_ = rhs.vk_features_;
  vk_properties_ = rhs.vk_properties_;
  vk_memory_properties_ = rhs.vk_memory_properties_;
  vk_queue_families_ = rhs.vk_queue_families_;
  return *this;
}

PhysicalDevice &PhysicalDevice::operator=(PhysicalDevice &&rhs) noexcept {
  vk_device_ = rhs.vk_device_;
  vk_extensions_ = std::move(rhs.vk_extensions_);
  vk_features_ = rhs.vk_features_;
  vk_properties_ = rhs.vk_properties_;
  vk_memory_properties_ = rhs.vk_memory_properties_;
  vk_queue_families_ = std::move(rhs.vk_queue_families_);
  return *this;
}

VeResult PhysicalDevice::setHandle(VkPhysicalDevice vk_physical_device) {
  clear();
  // set new info
  vk_device_ = vk_physical_device;
  VENUS_RETURN_BAD_RESULT(
      vk::checkAvailableExtensions(vk_device_, vk_extensions_));
  vkGetPhysicalDeviceFeatures(vk_device_, &vk_features_);
  vkGetPhysicalDeviceProperties(vk_device_, &vk_properties_);
  vkGetPhysicalDeviceMemoryProperties(vk_device_, &vk_memory_properties_);
  VENUS_RETURN_BAD_RESULT(
      vk::checkAvailableQueueFamilies(vk_device_, vk_queue_families_));
  return VeResult::noError();
}

void PhysicalDevice::clear() {
  vk_extensions_.clear();
  vk_features_ = {};
  vk_properties_ = {};
  vk_memory_properties_ = {};
  vk_queue_families_.clear();
}

VkPhysicalDevice PhysicalDevice::operator*() const { return vk_device_; }

PhysicalDevice::operator bool() const { return vk_device_ != VK_NULL_HANDLE; }

bool PhysicalDevice::isExtensionSupported(
    const char *desired_instance_extension) const {
  for (auto &extension : vk_extensions_)
    if (std::string(extension.extensionName) ==
        std::string(desired_instance_extension))
      return true;
  return false;
}

Result<u32> PhysicalDevice::selectIndexOfQueueFamily(
    VkQueueFlagBits desired_capabilities) const {
  for (u32 index = 0; index < static_cast<u32>(vk_queue_families_.size());
       ++index) {
    if ((vk_queue_families_[index].queueCount > 0) &&
        ((vk_queue_families_[index].queueFlags & desired_capabilities) ==
         desired_capabilities)) {
      return Result<u32>(index);
    }
  }
  return VeResult::notFound();
}

Result<u32> PhysicalDevice::selectIndexOfQueueFamily(
    VkSurfaceKHR presentation_surface) const {
  for (u32 index = 0; index < static_cast<u32>(vk_queue_families_.size());
       ++index) {
    VkBool32 presentation_supported = VK_FALSE;
    VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(
        vk_device_, index, presentation_surface, &presentation_supported);
    if ((VK_SUCCESS == result) && (VK_TRUE == presentation_supported)) {
      return Result<u32>(index);
    }
  }
  return VeResult::notFound();
}

VkFormatProperties PhysicalDevice::formatProperties(VkFormat format) const {
  VkFormatProperties properties;
  vkGetPhysicalDeviceFormatProperties(vk_device_, format, &properties);
  return properties;
}

VkImageFormatProperties PhysicalDevice::imageFormatProperties(
    VkFormat format, VkImageType type, VkImageTiling tiling,
    VkImageUsageFlags usage, VkImageCreateFlags flags) const {
  VkImageFormatProperties properties;
  vkGetPhysicalDeviceImageFormatProperties(vk_device_, format, type, tiling,
                                           usage, flags, &properties);
  return properties;
}

u32 PhysicalDevice::chooseMemoryType(
    const VkMemoryRequirements &memory_requirements,
    VkMemoryPropertyFlags required_flags,
    VkMemoryPropertyFlags preferred_flags) const {
  u32 selected_type = ~0u;
  u32 memory_type;
  for (memory_type = 0; memory_type < vk_memory_properties_.memoryTypeCount;
       ++memory_type) {
    if (memory_requirements.memoryTypeBits & (1 << memory_type)) {
      const VkMemoryType &type = vk_memory_properties_.memoryTypes[memory_type];
      if ((type.propertyFlags & preferred_flags) == preferred_flags) {
        selected_type = memory_type;
        break;
      }
    }
  }
  if (selected_type != ~0u) {
    for (memory_type = 0; memory_type < 32; ++memory_type) {
      if (memory_requirements.memoryTypeBits & (1 << memory_type)) {
        const VkMemoryType &type =
            vk_memory_properties_.memoryTypes[memory_type];
        if ((type.propertyFlags & required_flags) == required_flags) {
          selected_type = memory_type;
          break;
        }
      }
    }
  }
  return selected_type;
}

Result<VkPresentModeKHR> PhysicalDevice::selectPresentationMode(
    VkSurfaceKHR presentation_surface,
    VkPresentModeKHR desired_present_mode) const {
  // Enumerate supported present modes
  u32 present_modes_count = 0;
  VENUS_VK_RETURN_BAD_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(
      vk_device_, presentation_surface, &present_modes_count, nullptr));
  if (0 == present_modes_count) {
    HERMES_ERROR("Could not get the number of supported present modes.");
    return VeResult::notFound();
  }
  std::vector<VkPresentModeKHR> present_modes(present_modes_count);
  VENUS_VK_RETURN_BAD_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(
      vk_device_, presentation_surface, &present_modes_count,
      present_modes.data()));
  if (0 == present_modes_count) {
    HERMES_ERROR("Could not enumerate present modes.");
    return VeResult::notFound();
  }
  // Select present mode
  for (auto &current_present_mode : present_modes)
    if (current_present_mode == desired_present_mode)
      return Result<VkPresentModeKHR>(desired_present_mode);
  HERMES_INFO(
      "Desired present mode is not supported. Selecting default FIFO mode.");
  for (auto &current_present_mode : present_modes)
    if (current_present_mode == VK_PRESENT_MODE_FIFO_KHR)
      return Result<VkPresentModeKHR>(VK_PRESENT_MODE_FIFO_KHR);
  HERMES_ERROR(
      "VK_PRESENT_MODE_FIFO_KHR is not supported though it's mandatory "
      "for all drivers!");
  return VeResult::notFound();
}

VeResult PhysicalDevice::selectFormatOfSwapchainImages(
    VkSurfaceKHR presentation_surface,
    VkSurfaceFormatKHR desired_surface_format, VkFormat &image_format,
    VkColorSpaceKHR &image_color_space) const {
  // Enumerate supported formats
  u32 formats_count = 0;

  VENUS_VK_RETURN_BAD_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(
      vk_device_, presentation_surface, &formats_count, nullptr))
  if (0 == formats_count) {
    HERMES_ERROR("Could not get the number of supported surface formats.");
    return VeResult::notFound();
  }

  std::vector<VkSurfaceFormatKHR> surface_formats(formats_count);
  VENUS_VK_RETURN_BAD_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(
      vk_device_, presentation_surface, &formats_count,
      surface_formats.data()));
  if (0 == formats_count) {
    HERMES_ERROR("Could not enumerate supported surface formats.");
    return VeResult::notFound();
  }

  // Select surface format
  if ((1 == surface_formats.size()) &&
      (VK_FORMAT_UNDEFINED == surface_formats[0].format)) {
    image_format = desired_surface_format.format;
    image_color_space = desired_surface_format.colorSpace;
    return VeResult::noError();
  }

  for (auto &surface_format : surface_formats) {
    if (desired_surface_format.format == surface_format.format &&
        desired_surface_format.colorSpace == surface_format.colorSpace) {
      image_format = desired_surface_format.format;
      image_color_space = desired_surface_format.colorSpace;
      return VeResult::noError();
    }
  }

  for (auto &surface_format : surface_formats) {
    if (desired_surface_format.format == surface_format.format) {
      image_format = desired_surface_format.format;
      image_color_space = surface_format.colorSpace;
      HERMES_INFO("Desired combination of format and colorspace is not "
                  "supported. Selecting other colorspace.");
      return VeResult::noError();
    }
  }

  image_format = surface_formats[0].format;
  image_color_space = surface_formats[0].colorSpace;
  HERMES_INFO("Desired format is not supported. Selecting available format - "
              "colorspace combination.");
  return VeResult::noError();
}

Result<VkFormat>
PhysicalDevice::findSupportedFormat(const std::vector<VkFormat> &candidates,
                                    VkImageTiling tiling,
                                    VkFormatFeatureFlags features) const {
  for (VkFormat candidate_format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(vk_device_, candidate_format, &props);
    if (tiling == VK_IMAGE_TILING_LINEAR &&
        (props.linearTilingFeatures & features) == features) {
      return Result<VkFormat>(candidate_format);
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
               (props.optimalTilingFeatures & features) == features) {
      return Result<VkFormat>(candidate_format);
    }
  }
  HERMES_ERROR("Failed to find supported format.");
  return VeResult::notFound();
}

[[maybe_unused]] Result<VkSurfaceCapabilitiesKHR>
PhysicalDevice::surfaceCapabilities(VkSurfaceKHR surface) const {
  VkSurfaceCapabilitiesKHR surface_capabilities;
  VENUS_VK_RETURN_BAD_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      vk_device_, surface, &surface_capabilities));
  return Result<VkSurfaceCapabilitiesKHR>(surface_capabilities);
}

const VkPhysicalDeviceProperties &PhysicalDevice::properties() const {
  return vk_properties_;
}

const VkPhysicalDeviceFeatures &PhysicalDevice::features() const {
  return vk_features_;
}

[[maybe_unused]] VkSampleCountFlagBits
PhysicalDevice::maxUsableSampleCount(bool include_depth_buffer) const {
  VkSampleCountFlags counts =
      vk_properties_.limits.framebufferColorSampleCounts &
      (include_depth_buffer ? vk_properties_.limits.framebufferDepthSampleCounts
                            : 0);
  if (counts & VK_SAMPLE_COUNT_64_BIT)
    return VK_SAMPLE_COUNT_64_BIT;
  if (counts & VK_SAMPLE_COUNT_32_BIT)
    return VK_SAMPLE_COUNT_32_BIT;
  if (counts & VK_SAMPLE_COUNT_16_BIT)
    return VK_SAMPLE_COUNT_16_BIT;
  if (counts & VK_SAMPLE_COUNT_8_BIT)
    return VK_SAMPLE_COUNT_8_BIT;
  if (counts & VK_SAMPLE_COUNT_4_BIT)
    return VK_SAMPLE_COUNT_4_BIT;
  if (counts & VK_SAMPLE_COUNT_2_BIT)
    return VK_SAMPLE_COUNT_2_BIT;
  return VK_SAMPLE_COUNT_1_BIT;
}

std::ostream &operator<<(std::ostream &os, VkPhysicalDeviceType e) {
#define PRINT_IF_ENUM(E)                                                       \
  if (e == E)                                                                  \
    os << #E;
  PRINT_IF_ENUM(VK_PHYSICAL_DEVICE_TYPE_OTHER)
  PRINT_IF_ENUM(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
  PRINT_IF_ENUM(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
  PRINT_IF_ENUM(VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
  PRINT_IF_ENUM(VK_PHYSICAL_DEVICE_TYPE_CPU)
//  PRINT_IF_ENUM(VK_PHYSICAL_DEVICE_TYPE_BEGIN_RANGE);
//  PRINT_IF_ENUM(VK_PHYSICAL_DEVICE_TYPE_END_RANGE);
//  PRINT_IF_ENUM(VK_PHYSICAL_DEVICE_TYPE_RANGE_SIZE);
#undef PRINT_IF_ENUM
  return os;
}

std::ostream &operator<<(std::ostream &os, const PhysicalDevice &d) {
  auto properties = d.properties();
  os << "PHYSICAL DEVICE INFO =====================" << std::endl;
#define PRINT_FIELD(F) os << #F << " = " << F << "\n";
  PRINT_FIELD(properties.deviceName)
  PRINT_FIELD(properties.deviceType)
  PRINT_FIELD(properties.deviceID)
  PRINT_FIELD(properties.vendorID)
  PRINT_FIELD(properties.apiVersion)
  PRINT_FIELD(properties.driverVersion)
#undef PRINT_FIELD
  os << "==========================================" << std::endl;
  return os;
}

} // namespace venus::core

namespace venus {
HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::core::PhysicalDevice)
HERMES_PUSH_DEBUG_VK_HANDLE(vk_device_);
HERMES_PUSH_DEBUG_ARRAY_FIELD_BEGIN(vk_extensions_, extension);
// HERMES_PUSH_DEBUG_VK_VARIABLE(extension);
HERMES_PUSH_DEBUG_ARRAY_FIELD_END
// HERMES_PUSH_DEBUG_VK_FIELD(vk_features_);
// HERMES_PUSH_DEBUG_VK_FIELD(vk_properties_);
// HERMES_PUSH_DEBUG_VK_FIELD(vk_memory_properties_);
HERMES_PUSH_DEBUG_ARRAY_FIELD_BEGIN(vk_queue_families_, vk_queue_family);
// HERMES_PUSH_DEBUG_VK_VARIABLE(vk_queue_family);
HERMES_PUSH_DEBUG_ARRAY_FIELD_END
HERMES_TO_STRING_DEBUG_METHOD_END

} // namespace venus
