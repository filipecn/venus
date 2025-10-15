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
/// \brief  Vulkan Logical Device

#pragma once

#include <venus/core/physical_device.h>
#include <venus/utils/macros.h>

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include <vk_mem_alloc.h>

namespace venus::core {

/// The logical device makes the interface of the application and the physical
/// device. Represents the hardware, along with the extensions and features
/// enabled for it and all the queues requested from it. The logical device
/// allows us to record commands, submit them to queues and acquire the results.
/// Device queues need to be requested on device creation, we cannot create or
/// destroy queues explicitly. They are created/destroyed on logical device
/// creation/destruction.
/// \note This class uses RAII.
class Device {
public:
  /// Builder for Device class.
  struct Config {

    /// \param extension_name
    Config &addExtension(const std::string_view &extension_name);
    /// \param extension_names
    Config &addExtensions(const std::vector<std::string> &extension_names);
    /// \param features All device features.
    Config &setFeatures(const vk::DeviceFeatures &features);
    /// \param features Physical device features.
    Config &setFeatures(const VkPhysicalDeviceFeatures &features);
    /// \param features Physical device features2.
    Config &setFeatures2(const VkPhysicalDeviceFeatures2 &features);
    /// \param features Physical device vk 12 features.
    Config &
    setVulkan12Features(const VkPhysicalDeviceVulkan12Features &features);
    /// \param features Physical device vk 13 features.
    Config &
    setVulkan13Features(const VkPhysicalDeviceVulkan13Features &features);
    /// \param features Physical device indexing features.
    Config &setDescriptorIndexingFeatures(
        const VkPhysicalDeviceDescriptorIndexingFeaturesEXT &features);
    /// \param features Physical device synchronization2 features.
    Config &setSynchronization2Features(
        const VkPhysicalDeviceSynchronization2FeaturesKHR &features);
    /// \param flags
    Config &addCreateFlags(VkDeviceCreateFlags flags);
    /// \param flags
    Config &addAllocationFlags(VmaAllocatorCreateFlags flags);
    /// \note This indicates a family with size of queue_priorities elements.
    /// \note If the family index has already been added, this will append the
    ///       priorities to the previous priorities for that family index.
    /// \param index Queue family index (returned by physical device methods)
    /// \param queue_priorities Priorities the queues in the family.
    /// \param flags [def={}]
    Config &addQueueFamily(u32 index, const std::vector<f32> &queue_priorities,
                           VkDeviceQueueCreateFlags flags = {});

    /// Creates a new logical device with this configuration.
    /// \param physical_device
    /// \return The newly created device or error.
    HERMES_NODISCARD Result<Device>
    create(const PhysicalDevice &physical_device) const;

  private:
    vk::DeviceFeatures features_;
    std::vector<std::string> extensions_;
    std::vector<vk::QueueFamilyConfig> family_configs_;
    VkDeviceCreateFlags flags_;
    VmaAllocatorCreateInfo allocator_info_;

    VENUS_to_string_FRIEND(Device::Config);
  };

  // raii

  VENUS_DECLARE_RAII_FUNCTIONS(Device)

  /// Destroy underlying vulkan logical device object and clear all data.
  void destroy() noexcept;
  void swap(Device &rhs) noexcept;
  /// \return Associated physical device.
  const PhysicalDevice &physical() const;
  /// \return Underlying vulkan logical device object.
  VkDevice operator*() const;
  /// \return allocator
  VmaAllocator allocator() const;

protected:
  VmaAllocator allocator_{VK_NULL_HANDLE};
  VkDevice vk_device_{VK_NULL_HANDLE};
  PhysicalDevice physical_device_;

  VENUS_to_string_FRIEND(Device);
};

} // namespace venus::core
