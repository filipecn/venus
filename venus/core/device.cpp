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

/// \file   device.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30

#include <venus/core/device.h>

#include <venus/utils/vk_debug.h>

namespace venus::core {

Device::Config &
Device::Config::addExtension(const std::string_view &extension_name) {
  extensions_.emplace_back(extension_name);
  return *this;
}

Device::Config &
Device::Config::addExtensions(const std::vector<std::string> &extension_names) {
  extensions_.insert(extensions_.end(), extension_names.begin(),
                     extension_names.end());
  return *this;
}

Device::Config &
Device::Config::setFeatures(const vk::DeviceFeatures &features) {
  features_ = features;
  return *this;
}

Device::Config &
Device::Config::setFeatures(const VkPhysicalDeviceFeatures &features) {
  features_.f = features;
  return *this;
}

Device::Config &
Device::Config::setFeatures2(const VkPhysicalDeviceFeatures2 &features) {
  features_.f2 = features;
  return *this;
}

Device::Config &Device::Config::setVulkan12Features(
    const VkPhysicalDeviceVulkan12Features &features) {
  features_.v12_f = features;
  return *this;
}

Device::Config &Device::Config::setVulkan13Features(
    const VkPhysicalDeviceVulkan13Features &features) {
  features_.v13_f = features;
  return *this;
}

Device::Config &Device::Config::setDescriptorIndexingFeatures(
    const VkPhysicalDeviceDescriptorIndexingFeaturesEXT &features) {
  features_.descriptor_indexing_f = features;
  return *this;
}

Device::Config &Device::Config::setSynchronization2Features(
    const VkPhysicalDeviceSynchronization2FeaturesKHR &features) {
  features_.synchronization2_f = features;
  return *this;
}

Device::Config &Device::Config::addCreateFlags(VkDeviceCreateFlags flags) {
  flags_ |= flags;
  return *this;
}

Device::Config &
Device::Config::addAllocationFlags(VmaAllocatorCreateFlags flags) {
  allocator_info_.flags |= flags;
  return *this;
}

Device::Config &
Device::Config::addQueueFamily(u32 index,
                               const std::vector<f32> &queue_priorities,
                               VkDeviceQueueCreateFlags flags) {
  for (auto &family : family_configs_) {
    if (family.index == index) {
      family.priorities.insert(family.priorities.end(),
                               queue_priorities.begin(),
                               queue_priorities.end());
      return *this;
    }
  }
  family_configs_.push_back(
      {.index = index, .priorities = queue_priorities, .flags = flags});
  return *this;
}

Result<Device>
Device::Config::create(const PhysicalDevice &physical_device) const {
  std::vector<char const *> extensions;
  extensions.reserve(extensions_.size());
  for (auto const &ext : extensions_) {
    extensions.push_back(ext.data());
  }

  auto features = features_;
  features.v13_f.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
  features.v12_f.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;

  features.v12_f.pNext = &features.v13_f;
  features.f2.pNext = &features.v12_f;

  std::vector<VkDeviceQueueCreateInfo> queue_infos;
  for (const auto &family_config : family_configs_) {
    VkDeviceQueueCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = family_config.flags;
    info.queueFamilyIndex = family_config.index;
    info.queueCount = family_config.priorities.size();
    info.pQueuePriorities = family_config.priorities.data();
    queue_infos.push_back(info);
  }

  VkDeviceCreateInfo create_info{};
  create_info.pNext = &features.f2;
  create_info.flags = flags_;
  create_info.pQueueCreateInfos = queue_infos.data();
  create_info.queueCreateInfoCount = queue_infos.size();
  create_info.ppEnabledExtensionNames = extensions.data();
  create_info.enabledExtensionCount = extensions.size();

  Device device;
  device.physical_device_ = physical_device;
  VENUS_VK_RETURN_BAD_RESULT(vkCreateDevice(*physical_device, &create_info,
                                            nullptr, &device.vk_device_));

  VmaVulkanFunctions vulkan_functions = {};
  vulkan_functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
  vulkan_functions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

  VmaAllocatorCreateInfo allocator_info = allocator_info_;
  allocator_info.device = device.vk_device_;
  allocator_info.physicalDevice = *device.physical_device_;
  allocator_info.instance = device.physical_device_.instance();
  allocator_info.pVulkanFunctions = &vulkan_functions;

  VENUS_VK_RETURN_BAD_RESULT(
      vmaCreateAllocator(&allocator_info, &device.allocator_));

  return Result<Device>(std::move(device));
}

Device::Device(Device &&rhs) noexcept { *this = std::move(rhs); }

Device::~Device() noexcept { destroy(); }

Device &Device::operator=(Device &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void Device::swap(Device &rhs) noexcept {
  VENUS_SWAP_FIELD_WITH_RHS(vk_device_);
  VENUS_SWAP_FIELD_WITH_RHS(allocator_);
  VENUS_SWAP_FIELD_WITH_RHS(physical_device_);
}

void Device::destroy() noexcept {
  if (allocator_) {
    vmaDestroyAllocator(allocator_);
    allocator_ = VK_NULL_HANDLE;
  }
  if (vk_device_) {
    vkDestroyDevice(vk_device_, nullptr);
    vk_device_ = VK_NULL_HANDLE;
  }
}

VkDevice Device::operator*() const { return vk_device_; }

VmaAllocator Device::allocator() const { return allocator_; }

const PhysicalDevice &Device::physical() const { return physical_device_; }

} // namespace venus::core

namespace venus {
HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::core::Device::Config)
HERMES_PUSH_DEBUG_TITLE
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::core::Device)
HERMES_PUSH_DEBUG_TITLE
HERMES_TO_STRING_DEBUG_METHOD_END

} // namespace venus
