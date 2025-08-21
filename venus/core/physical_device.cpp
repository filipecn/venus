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

#include <venus/core/physical_device.h>

#include <venus/utils/vk_debug.h>

namespace venus::core {

PhysicalDevice::PhysicalDevice(VkPhysicalDevice handle, VkInstance instance) {
  if (!setHandle(handle, instance))
    return;
}

PhysicalDevice::PhysicalDevice(PhysicalDevice &&rhs) noexcept {
  *this = std::move(rhs);
  rhs.clear();
}

PhysicalDevice::PhysicalDevice(const PhysicalDevice &rhs) { *this = rhs; }

PhysicalDevice &PhysicalDevice::operator=(const PhysicalDevice &rhs) {
  vk_physical_device_ = rhs.vk_physical_device_;
  vk_instance_ = rhs.vk_instance_;
  vk_extensions_ = rhs.vk_extensions_;
  vk_features_ = rhs.vk_features_;
  vk_properties_ = rhs.vk_properties_;
  vk_memory_properties_ = rhs.vk_memory_properties_;
  vk_queue_families_ = rhs.vk_queue_families_;
  return *this;
}

PhysicalDevice &PhysicalDevice::operator=(PhysicalDevice &&rhs) noexcept {
  vk_physical_device_ = rhs.vk_physical_device_;
  vk_instance_ = rhs.vk_instance_;
  vk_extensions_ = std::move(rhs.vk_extensions_);
  vk_features_ = rhs.vk_features_;
  vk_properties_ = rhs.vk_properties_;
  vk_memory_properties_ = rhs.vk_memory_properties_;
  vk_queue_families_ = std::move(rhs.vk_queue_families_);
  return *this;
}

VeResult PhysicalDevice::setHandle(VkPhysicalDevice vk_physical_device,
                                   VkInstance vk_instance) {
  clear();
  // set new info
  vk_physical_device_ = vk_physical_device;
  vk_instance_ = vk_instance;
  if (!vk_physical_device_)
    return VeResult::noError();
  VENUS_RETURN_BAD_RESULT(
      vk::checkAvailableExtensions(vk_physical_device_, vk_extensions_));
  vkGetPhysicalDeviceFeatures(vk_physical_device_, &vk_features_);
  vkGetPhysicalDeviceProperties(vk_physical_device_, &vk_properties_);
  vkGetPhysicalDeviceMemoryProperties(vk_physical_device_,
                                      &vk_memory_properties_);
  VENUS_RETURN_BAD_RESULT(
      vk::checkAvailableQueueFamilies(vk_physical_device_, vk_queue_families_));
  return VeResult::noError();
}

VkInstance PhysicalDevice::instance() const { return vk_instance_; }

void PhysicalDevice::clear() {
  vk_extensions_.clear();
  vk_features_ = {};
  vk_properties_ = {};
  vk_memory_properties_ = {};
  vk_queue_families_.clear();
}

VkPhysicalDevice PhysicalDevice::operator*() const {
  return vk_physical_device_;
}

PhysicalDevice::operator bool() const {
  return vk_instance_ && vk_physical_device_;
}

bool PhysicalDevice::isExtensionSupported(
    const char *desired_instance_extension) const {
  for (auto &extension : vk_extensions_)
    if (std::string(extension.extensionName) ==
        std::string(desired_instance_extension))
      return true;
  return false;
}

Result<u32> PhysicalDevice::selectIndexOfQueueFamily(
    VkQueueFlags desired_capabilities) const {
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
        vk_physical_device_, index, presentation_surface,
        &presentation_supported);
    if ((VK_SUCCESS == result) && (VK_TRUE == presentation_supported)) {
      return Result<u32>(index);
    }
  }
  return VeResult::notFound();
}

Result<vk::GraphicsQueueFamilyIndices>
PhysicalDevice::selectGraphicsQueueFamilyIndices(
    VkSurfaceKHR vk_presentation_surface) const {
  vk::GraphicsQueueFamilyIndices indices;
  VENUS_ASSIGN_RESULT_OR_RETURN(indices.graphics_queue_family_index,
                                selectIndexOfQueueFamily(VK_QUEUE_GRAPHICS_BIT),
                                VeResult::notFound());
  VENUS_ASSIGN_RESULT_OR_RETURN(
      indices.present_queue_family_index,
      selectIndexOfQueueFamily(vk_presentation_surface), VeResult::notFound());
  return Result<vk::GraphicsQueueFamilyIndices>(std::move(indices));
}

VkFormatProperties PhysicalDevice::formatProperties(VkFormat format) const {
  VkFormatProperties properties;
  vkGetPhysicalDeviceFormatProperties(vk_physical_device_, format, &properties);
  return properties;
}

VkImageFormatProperties PhysicalDevice::imageFormatProperties(
    VkFormat format, VkImageType type, VkImageTiling tiling,
    VkImageUsageFlags usage, VkImageCreateFlags flags) const {
  VkImageFormatProperties properties;
  vkGetPhysicalDeviceImageFormatProperties(vk_physical_device_, format, type,
                                           tiling, usage, flags, &properties);
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
      vk_physical_device_, presentation_surface, &present_modes_count,
      nullptr));
  if (0 == present_modes_count) {
    HERMES_ERROR("Could not get the number of supported present modes.");
    return VeResult::notFound();
  }
  std::vector<VkPresentModeKHR> present_modes(present_modes_count);
  VENUS_VK_RETURN_BAD_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(
      vk_physical_device_, presentation_surface, &present_modes_count,
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

Result<VkSurfaceFormatKHR> PhysicalDevice::selectFormatOfSwapchainImages(
    VkSurfaceKHR presentation_surface,
    VkSurfaceFormatKHR desired_surface_format) const {
  // Enumerate supported formats
  u32 candidate_count = 0;

  VENUS_VK_RETURN_BAD_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(
      vk_physical_device_, presentation_surface, &candidate_count, nullptr))
  if (0 == candidate_count) {
    HERMES_ERROR("Could not get the number of supported surface formats.");
    return VeResult::notFound();
  }

  std::vector<VkSurfaceFormatKHR> candidates(candidate_count);
  VENUS_VK_RETURN_BAD_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(
      vk_physical_device_, presentation_surface, &candidate_count,
      candidates.data()));
  if (0 == candidate_count) {
    HERMES_ERROR("Could not enumerate supported surface formats.");
    return VeResult::notFound();
  }

  VkSurfaceFormatKHR image_format;
  // Select surface format
  if (1 == candidate_count) {
    if (candidates[0].format == VK_FORMAT_UNDEFINED) {
      image_format.format = VK_FORMAT_B8G8R8A8_UNORM;
      image_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    } else {
      image_format = candidates[0];
    }
    return Result<VkSurfaceFormatKHR>(image_format);
  }

  for (auto &candidate : candidates)
    if (desired_surface_format.format == candidate.format &&
        desired_surface_format.colorSpace == candidate.colorSpace)
      return Result<VkSurfaceFormatKHR>(candidate);

  for (auto &candidate : candidates)
    if (desired_surface_format.format == candidate.format) {
      image_format.format = desired_surface_format.format;
      image_format.colorSpace = candidate.colorSpace;
      HERMES_INFO("Desired combination of format and colorspace is not "
                  "supported. Selecting other colorspace.");
      return Result<VkSurfaceFormatKHR>(image_format);
    }

  image_format.format = candidates[0].format;
  image_format.colorSpace = candidates[0].colorSpace;
  HERMES_INFO("Desired swapchain surface (format, colorspace) ({}, {}) is not "
              "supported.",
              string_VkFormat(desired_surface_format.format),
              string_VkColorSpaceKHR(desired_surface_format.colorSpace));
  HERMES_INFO("Selecting available pair ({}, {}) ",
              string_VkFormat(image_format.format),
              string_VkColorSpaceKHR(image_format.colorSpace));
  return Result<VkSurfaceFormatKHR>(image_format);
}

Result<VkFormat>
PhysicalDevice::findSupportedFormat(const std::vector<VkFormat> &candidates,
                                    VkImageTiling tiling,
                                    VkFormatFeatureFlags features) const {
  for (VkFormat candidate_format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(vk_physical_device_, candidate_format,
                                        &props);
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
      vk_physical_device_, surface, &surface_capabilities));
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

PhysicalDevices::Selector &
PhysicalDevices::Selector::forGraphics(VkSurfaceKHR _surface) {
  surface = _surface;
  queue_flags |= VK_QUEUE_GRAPHICS_BIT;
  return *this;
}

PhysicalDevices::Selector &
PhysicalDevices::Selector::setSurface(VkSurfaceKHR _surface) {
  surface = _surface;
  return *this;
}

PhysicalDevices::Selector &
PhysicalDevices::Selector::setFeatures(const vk::DeviceFeatures &features) {
  device_features = features;
  return *this;
}

PhysicalDevices::Selector &PhysicalDevices::Selector::setFeatures(
    const VkPhysicalDeviceFeatures &features) {
  device_features.f = features;
  return *this;
}

PhysicalDevices::Selector &PhysicalDevices::Selector::setFeatures2(
    const VkPhysicalDeviceFeatures2 &features) {
  device_features.f2 = features;
  return *this;
}

PhysicalDevices::Selector &PhysicalDevices::Selector::setVulkan12Features(
    const VkPhysicalDeviceVulkan12Features &features) {
  device_features.v12_f = features;
  return *this;
}

PhysicalDevices::Selector &PhysicalDevices::Selector::setVulkan13Features(
    const VkPhysicalDeviceVulkan13Features &features) {
  device_features.v13_f = features;
  return *this;
}

PhysicalDevices::Selector &
PhysicalDevices::Selector::setDescriptorIndexingFeatures(
    const VkPhysicalDeviceDescriptorIndexingFeaturesEXT &features) {
  device_features.descriptor_indexing_f = features;
  return *this;
}

PhysicalDevices::Selector &
PhysicalDevices::Selector::setSynchronization2Features(
    const VkPhysicalDeviceSynchronization2FeaturesKHR &features) {
  device_features.synchronization2_f = features;
  return *this;
}

PhysicalDevices::Selector &
PhysicalDevices::Selector::addQueueFlags(VkQueueFlags flags) {
  queue_flags |= flags;
  return *this;
}

Result<PhysicalDevice>
PhysicalDevices::select(const PhysicalDevices::Selector &selector) const {
  for (const auto &physical_device : *this) {
    // TODO check features
    if (!physical_device.selectIndexOfQueueFamily(selector.queue_flags))
      continue;
    if (selector.surface &&
        !physical_device.selectIndexOfQueueFamily(selector.surface))
      continue;
    return Result<PhysicalDevice>(physical_device);
  }
  return VeResult::notFound();
}

} // namespace venus::core

std::string decodeAPIVersion(uint32_t apiVersion) {
  return std::to_string(VK_VERSION_MAJOR(apiVersion)) + "." +
         std::to_string(VK_VERSION_MINOR(apiVersion)) + "." +
         std::to_string(VK_VERSION_PATCH(apiVersion));
}

std::string decodeDriverVersion(uint32_t driver_version, uint32_t vendor_id) {
  switch (vendor_id) {
  case 0x10DE:
    return std::to_string((driver_version >> 22) & 0x3FF) + "." +
           std::to_string((driver_version >> 14) & 0xFF) + "." +
           std::to_string((driver_version >> 6) & 0xFF) + "." +
           std::to_string(driver_version & 0x3F);
  case 0x8086:
    return std::to_string((driver_version >> 14) & 0x3FFFF) + "." +
           std::to_string((driver_version & 0x3FFF));
  default:
    return decodeAPIVersion(driver_version);
  }
}

std::string decodeVendorID(uint32_t id) {
  // below 0x10000 are the PCI vendor IDs
  // (https://pcisig.com/membership/member-companies)
  if (id < 0x10000) {
    switch (id) {
    case 0x1022:
      return "Advanced Micro Devices";
    case 0x10DE:
      return "NVidia Corporation";
    case 0x8086:
      return "Intel Corporation";
    default:
      return std::to_string(id);
    }
  } else {
    // above 0x10000 should be vkVendorIDs
    // TODO: return vk::to_string(vk::VendorId(id));
    return "unknown";
  }
}

namespace venus {
HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::core::PhysicalDevice)
HERMES_PUSH_DEBUG_LINE("Name:           {}", object.vk_properties_.deviceName);
HERMES_PUSH_DEBUG_LINE(
    "Type:           {}",
    string_VkPhysicalDeviceType(object.vk_properties_.deviceType));
HERMES_PUSH_DEBUG_LINE("API Version:    {}",
                       decodeAPIVersion(object.vk_properties_.apiVersion));
HERMES_PUSH_DEBUG_LINE("Driver Version: {}",
                       decodeDriverVersion(object.vk_properties_.driverVersion,
                                           object.vk_properties_.vendorID));
HERMES_PUSH_DEBUG_LINE("Vendor ID:      {}",
                       decodeVendorID(object.vk_properties_.vendorID));
HERMES_PUSH_DEBUG_LINE("Device ID:      {}", object.vk_properties_.deviceID);
HERMES_PUSH_DEBUG_LINE("#Family Queues: {}", object.vk_extensions_.size());
// HERMES_PUSH_DEBUG_ARRAY_FIELD_BEGIN(vk_queue_families_, vk_queue_family);
// HERMES_PUSH_DEBUG_LINE("{}",
// string_VkQueueFlags(vk_queue_family.queueFlags));
// HERMES_PUSH_DEBUG_ARRAY_FIELD_END
HERMES_PUSH_DEBUG_LINE("#extensions:    {}", object.vk_extensions_.size());
// HERMES_PUSH_DEBUG_ARRAY_FIELD_BEGIN(vk_extensions_, extension);
// HERMES_PUSH_DEBUG_VK_VARIABLE(extension.extensionName);
// HERMES_PUSH_DEBUG_ARRAY_FIELD_END
HERMES_PUSH_DEBUG_VK_HANDLE(vk_physical_device_);
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::core::PhysicalDevices)
HERMES_PUSH_DEBUG_LINE("#devices: {}", object.size());
for (const auto &physical_device : object) {
  HERMES_PUSH_DEBUG_LINE("device[]: \n{}", venus::to_string(physical_device));
}
HERMES_TO_STRING_DEBUG_METHOD_END

} // namespace venus
