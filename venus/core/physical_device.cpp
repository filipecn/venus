/// Copyright (c) 2025, FilipeCN.
///
/// The MIT License (MIT)
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///
///\file vulkan_physical_device.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2025-06-07
///
///\brief

#include <venus/core/debug.h>
#include <venus/core/physical_device.h>

namespace venus::core {

PhysicalDevice::PhysicalDevice() = default;

PhysicalDevice::PhysicalDevice(VkPhysicalDevice device_handle) {
  setHandle(device_handle);
}

PhysicalDevice::PhysicalDevice(PhysicalDevice &&other) noexcept {
  vk_device_ = other.vk_device_;
  vk_extensions_ = std::move(other.vk_extensions_);
  vk_features_ = other.vk_features_;
  vk_properties_ = other.vk_properties_;
  vk_memory_properties_ = other.vk_memory_properties_;
  vk_queue_families_ = std::move(other.vk_queue_families_);
}

PhysicalDevice::PhysicalDevice(const PhysicalDevice &other) {
  vk_device_ = other.vk_device_;
  vk_extensions_ = other.vk_extensions_;
  vk_features_ = other.vk_features_;
  vk_properties_ = other.vk_properties_;
  vk_memory_properties_ = other.vk_memory_properties_;
  vk_queue_families_ = other.vk_queue_families_;
}

PhysicalDevice &PhysicalDevice::operator=(const PhysicalDevice &other) {
  vk_device_ = other.vk_device_;
  vk_extensions_ = other.vk_extensions_;
  vk_features_ = other.vk_features_;
  vk_properties_ = other.vk_properties_;
  vk_memory_properties_ = other.vk_memory_properties_;
  vk_queue_families_ = other.vk_queue_families_;
  return *this;
}

PhysicalDevice &PhysicalDevice::operator=(PhysicalDevice &&other) noexcept {
  vk_device_ = other.vk_device_;
  vk_extensions_ = std::move(other.vk_extensions_);
  vk_features_ = other.vk_features_;
  vk_properties_ = other.vk_properties_;
  vk_memory_properties_ = other.vk_memory_properties_;
  vk_queue_families_ = std::move(other.vk_queue_families_);
  return *this;
}

void PhysicalDevice::setHandle(VkPhysicalDevice device_handle) {
  // cleanup
  vk_extensions_.clear();
  vk_features_ = {};
  vk_properties_ = {};
  vk_memory_properties_ = {};
  vk_queue_families_.clear();
  // set new info
  vk_device_ = device_handle;
  if (!checkAvailableExtensions(vk_extensions_))
    return;
  vkGetPhysicalDeviceFeatures(vk_device_, &vk_features_);
  vkGetPhysicalDeviceProperties(vk_device_, &vk_properties_);
  vkGetPhysicalDeviceMemoryProperties(vk_device_, &vk_memory_properties_);
  checkAvailableQueueFamilies();
}

VkPhysicalDevice PhysicalDevice::handle() const { return vk_device_; }

bool PhysicalDevice::good() const { return vk_device_ != VK_NULL_HANDLE; }

bool PhysicalDevice::isExtensionSupported(
    const char *desired_instance_extension) const {
  for (auto &extension : vk_extensions_)
    if (std::string(extension.extensionName) ==
        std::string(desired_instance_extension))
      return true;
  return false;
}

bool PhysicalDevice::checkAvailableQueueFamilies() {
  u32 queue_families_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(vk_device_, &queue_families_count,
                                           nullptr);
  if (queue_families_count == 0) {
    VENUS_ERROR("Could not get the number of queue families.\n");
    return false;
  }
  vk_queue_families_.resize(queue_families_count);
  vkGetPhysicalDeviceQueueFamilyProperties(vk_device_, &queue_families_count,
                                           vk_queue_families_.data());
  if (queue_families_count == 0) {
    VENUS_ERROR("Could not acquire properties of queue families.\n");
    return false;
  }
  return true;
}

bool PhysicalDevice::selectIndexOfQueueFamily(
    VkQueueFlagBits desired_capabilities, u32 &queue_family_index) const {
  for (u32 index = 0; index < static_cast<u32>(vk_queue_families_.size());
       ++index) {
    if ((vk_queue_families_[index].queueCount > 0) &&
        ((vk_queue_families_[index].queueFlags & desired_capabilities) ==
         desired_capabilities)) {
      queue_family_index = index;
      return true;
    }
  }
  return false;
}

bool PhysicalDevice::selectIndexOfQueueFamily(VkSurfaceKHR presentation_surface,
                                              u32 &queue_family_index) const {
  for (u32 index = 0; index < static_cast<u32>(vk_queue_families_.size());
       ++index) {
    VkBool32 presentation_supported = VK_FALSE;
    VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(
        vk_device_, index, presentation_surface, &presentation_supported);
    if ((VK_SUCCESS == result) && (VK_TRUE == presentation_supported)) {
      queue_family_index = index;
      return true;
    }
  }
  return false;
}

bool PhysicalDevice::checkAvailableExtensions(
    std::vector<VkExtensionProperties> &extensions) const {
  u32 extensions_count = 0;
  R_CHECK_VULKAN(vkEnumerateDeviceExtensionProperties(
                     vk_device_, nullptr, &extensions_count, nullptr),
                 false)
  if (extensions_count == 0)
    return true;
  extensions.resize(extensions_count);
  R_CHECK_VULKAN(vkEnumerateDeviceExtensionProperties(
                     vk_device_, nullptr, &extensions_count, extensions.data()),
                 false)
  return true;
}

bool PhysicalDevice::formatProperties(VkFormat format,
                                      VkFormatProperties &properties) const {
  vkGetPhysicalDeviceFormatProperties(vk_device_, format, &properties);
  return true;
}

bool PhysicalDevice::imageFormatProperties(
    VkFormat format, VkImageType type, VkImageTiling tiling,
    VkImageUsageFlags usage, VkImageCreateFlags flags,
    VkImageFormatProperties &properties) const {
  vkGetPhysicalDeviceImageFormatProperties(vk_device_, format, type, tiling,
                                           usage, flags, &properties);
  return true;
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
bool PhysicalDevice::selectPresentationMode(
    VkSurfaceKHR presentation_surface, VkPresentModeKHR desired_present_mode,
    VkPresentModeKHR &present_mode) const {
  // Enumerate supported present modes
  u32 present_modes_count = 0;
  R_CHECK_VULKAN(
      vkGetPhysicalDeviceSurfacePresentModesKHR(
          vk_device_, presentation_surface, &present_modes_count, nullptr),
      false);
  if (0 == present_modes_count) {
    VENUS_ERROR("Could not get the number of supported present modes.");
    return false;
  }
  std::vector<VkPresentModeKHR> present_modes(present_modes_count);
  R_CHECK_VULKAN(vkGetPhysicalDeviceSurfacePresentModesKHR(
                     vk_device_, presentation_surface, &present_modes_count,
                     present_modes.data()),
                 false);
  if (0 == present_modes_count) {
    VENUS_ERROR("Could not enumerate present modes.");
    return false;
  }
  // Select present mode
  for (auto &current_present_mode : present_modes) {
    if (current_present_mode == desired_present_mode) {
      present_mode = desired_present_mode;
      return true;
    }
  }
  VENUS_INFO(
      "Desired present mode is not supported. Selecting default FIFO mode.");
  for (auto &current_present_mode : present_modes) {
    if (current_present_mode == VK_PRESENT_MODE_FIFO_KHR) {
      present_mode = VK_PRESENT_MODE_FIFO_KHR;
      return true;
    }
  }
  VENUS_INFO("VK_PRESENT_MODE_FIFO_KHR is not supported though it's mandatory "
             "for all drivers!");
  return false;
}
bool PhysicalDevice::selectFormatOfSwapchainImages(
    VkSurfaceKHR presentation_surface,
    VkSurfaceFormatKHR desired_surface_format, VkFormat &image_format,
    VkColorSpaceKHR &image_color_space) const {
  // Enumerate supported formats
  u32 formats_count = 0;

  R_CHECK_VULKAN(vkGetPhysicalDeviceSurfaceFormatsKHR(
                     vk_device_, presentation_surface, &formats_count, nullptr),
                 false)
  if (0 == formats_count) {
    VENUS_ERROR("Could not get the number of supported surface formats.");
    return false;
  }

  std::vector<VkSurfaceFormatKHR> surface_formats(formats_count);
  R_CHECK_VULKAN(vkGetPhysicalDeviceSurfaceFormatsKHR(
                     vk_device_, presentation_surface, &formats_count,
                     surface_formats.data()),
                 false)
  if (0 == formats_count) {
    VENUS_ERROR("Could not enumerate supported surface formats.");
    return false;
  }

  // Select surface format
  if ((1 == surface_formats.size()) &&
      (VK_FORMAT_UNDEFINED == surface_formats[0].format)) {
    image_format = desired_surface_format.format;
    image_color_space = desired_surface_format.colorSpace;
    return true;
  }

  for (auto &surface_format : surface_formats) {
    if (desired_surface_format.format == surface_format.format &&
        desired_surface_format.colorSpace == surface_format.colorSpace) {
      image_format = desired_surface_format.format;
      image_color_space = desired_surface_format.colorSpace;
      return true;
    }
  }

  for (auto &surface_format : surface_formats) {
    if (desired_surface_format.format == surface_format.format) {
      image_format = desired_surface_format.format;
      image_color_space = surface_format.colorSpace;
      VENUS_INFO("Desired combination of format and colorspace is not "
                 "supported. Selecting other colorspace.");
      return true;
    }
  }

  image_format = surface_formats[0].format;
  image_color_space = surface_formats[0].colorSpace;
  VENUS_INFO("Desired format is not supported. Selecting available format - "
             "colorspace combination.");
  return true;
}
bool PhysicalDevice::findSupportedFormat(
    const std::vector<VkFormat> &candidates, VkImageTiling tiling,
    VkFormatFeatureFlags features, VkFormat &format) const {
  for (VkFormat candidate_format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(vk_device_, candidate_format, &props);
    if (tiling == VK_IMAGE_TILING_LINEAR &&
        (props.linearTilingFeatures & features) == features) {
      format = candidate_format;
      return true;
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
               (props.optimalTilingFeatures & features) == features) {
      format = candidate_format;
      return true;
    }
  }
  VENUS_INFO("Failed to find supported format.");
  return false;
}
[[maybe_unused]] bool PhysicalDevice::surfaceCapabilities(
    VkSurfaceKHR surface,
    VkSurfaceCapabilitiesKHR &surface_capabilities) const {
  R_CHECK_VULKAN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                     vk_device_, surface, &surface_capabilities),
                 false)
  return true;
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
