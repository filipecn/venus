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

/// \file   physical_device.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2019-10-18
/// \brief  Vulkan Physical Device

#pragma once

#include <venus/core/vk_api.h>

#include <vector>

namespace venus::core {

/// Physical devices are the hardware we intend to use with Vulkan. Thus we
/// need to look for the devices that supports the features we need. We can
/// select any number of graphics cards and use them simultaneously.
/// The Vulkan Library allows us to get devices capabilities and properties so
/// we can select the one that best suits for our application.
///
/// Every Vulkan operation requires commands that are submitted to a queue.
/// Different queues can be processed independently and may support different
/// types of operations.
/// Queues with the same capabilities are grouped into families. A device may
/// expose any number of queue families. For example, there could be a queue
/// family that only allows processing of compute commands or one that only
/// allows memory transfer related commands.
class PhysicalDevice {
public:
  PhysicalDevice(VkPhysicalDevice handle);
  PhysicalDevice(PhysicalDevice &&rhs) noexcept;
  PhysicalDevice(const PhysicalDevice &rhs);
  PhysicalDevice &operator=(PhysicalDevice &&rhs) noexcept;
  PhysicalDevice &operator=(const PhysicalDevice &rhs);
  ~PhysicalDevice() = default;

  /// \return VkPhysicalDevice vulkan handle.
  HERMES_NODISCARD VkPhysicalDevice operator*() const;
  /// \return True if this object is valid.
  HERMES_NODISCARD operator bool() const;
  /// \param vk_physical_device Physical device vulkan handle.
  HERMES_NODISCARD VeResult setHandle(VkPhysicalDevice vk_physical_device);
  /// Clear data.
  void clear();

  /// \param  desired_device_extension extension name (ex: ).
  /// \return True if extension is supported by this device.
  bool isExtensionSupported(const char *desired_device_extension) const;

  /// Finds the queue family index that supports the desired set of
  /// capabilities.
  /// \param  desired_capabilities desired set of capabalities.
  /// \return A capable queue family index if found, error otherwise.
  HERMES_NODISCARD Result<u32>
  selectIndexOfQueueFamily(VkQueueFlagBits desired_capabilities) const;
  /// Finds a queue family of a physical device that can accept commands for a
  /// given surface.
  /// \param  vk_presentation_surface surface handle.
  /// \return A capable queue if found, error otherwise.
  HERMES_NODISCARD Result<u32>
  selectIndexOfQueueFamily(VkSurfaceKHR vk_presentation_surface) const;
  /// Gets the properties and level of support for a given format.
  /// \param vk_format
  VkFormatProperties formatProperties(VkFormat vk_format) const;
  /// Reports support for the format
  /// \param format
  /// \param type       ex: VK_IMAGE_TYPE_[1D, 2D, 3D]
  /// \param tiling     ex: VK_IMAGE_TILING_[LINEAR, OPTIMAL]
  /// \param usage
  /// \param flags
  /// \param properties
  VkImageFormatProperties imageFormatProperties(VkFormat format,
                                                VkImageType type,
                                                VkImageTiling tiling,
                                                VkImageUsageFlags usage,
                                                VkImageCreateFlags flags) const;
  /// Selects the index of memory type trying to satisfy the preferred
  /// requirements.
  /// \param memory_requirements  memory requirements for a particular
  ///  resource.
  /// \param  required_flags  hard requirements.
  /// \param  preferred_flags soft requirements.
  /// \return u32 memory type.
  HERMES_NODISCARD u32
  chooseMemoryType(const VkMemoryRequirements &memory_requirements,
                   VkMemoryPropertyFlags required_flags,
                   VkMemoryPropertyFlags preferred_flags) const;
  /// Checks if the desired presentation mode is supported by the device, if
  /// so, it is returned. If not, VK_PRESENT_MODE_FIFO_KHR is returned instead.
  /// \param  presentation_surface  surface handle.
  /// \param  desired_present_mode  described presentation mode.
  /// \return selected presentation mode, error otherwise.
  HERMES_NODISCARD
  Result<VkPresentModeKHR>
  selectPresentationMode(VkSurfaceKHR presentation_surface,
                         VkPresentModeKHR desired_present_mode) const;
  /// Clamps the desired image format to the supported format by the
  /// device. The format defines the number of color components, the number of
  /// bits for each component and data type. Also, we must specify the color
  /// space to be used for encoding color.
  /// \param[in]  presentation_surface    surface handle.
  /// \param[in]  desired_surface_format  desired image format.
  /// \param[out] image_format            receives available image format.
  /// \param[out] image_color_space       receives available color space.
  /// \return error status.
  HERMES_NODISCARD VeResult selectFormatOfSwapchainImages(
      VkSurfaceKHR presentation_surface,
      VkSurfaceFormatKHR desired_surface_format, VkFormat &image_format,
      VkColorSpaceKHR &image_color_space) const;
  /// Takes a list of candidate formats in order from most desirable to
  /// least desirable, and checks which is the first one that is supported
  /// \param  candidates
  /// \param  tiling
  /// \param  features
  /// \param  format
  /// \return found format, error otherwise.
  HERMES_NODISCARD Result<VkFormat>
  findSupportedFormat(const std::vector<VkFormat> &candidates,
                      VkImageTiling tiling,
                      VkFormatFeatureFlags features) const;
  ///
  [[maybe_unused]] HERMES_NODISCARD Result<VkSurfaceCapabilitiesKHR>
  surfaceCapabilities(VkSurfaceKHR surface) const;
  ///
  HERMES_NODISCARD const VkPhysicalDeviceProperties &properties() const;
  ///
  HERMES_NODISCARD const VkPhysicalDeviceFeatures &features() const;
  /// \param  include_depth_buffer **[def = true]** if true, computes
  ///         the highest sample count supported by both the color and depth
  ///         buffers
  /// \return [VkSampleCountFlagBits] the highest sample count supported by the
  ///         color buffer
  [[maybe_unused]] HERMES_NODISCARD VkSampleCountFlagBits
  maxUsableSampleCount(bool include_depth_buffer = true) const;

  VENUS_TO_STRING_FRIEND(PhysicalDevice);

private:
  /// Vulkan handle
  VkPhysicalDevice vk_device_{VK_NULL_HANDLE};
  /// Available device extensions
  std::vector<VkExtensionProperties> vk_extensions_;
  /// Device features include items such as geometry and tesselation shaders,
  /// depth clamp, etc.
  VkPhysicalDeviceFeatures vk_features_{};
  /// Device properties describe general information such as name, version of
  /// a driver, type of the device (integrated or discrete), memory, etc.
  VkPhysicalDeviceProperties vk_properties_{};
  /// Physical device memory properties such as number of heaps, sizes, types
  /// and etc.
  VkPhysicalDeviceMemoryProperties vk_memory_properties_{};
  std::vector<VkQueueFamilyProperties> vk_queue_families_;

  VENUS_TO_STRING_FRIEND(PhysicalDevice);
};

} // namespace venus::core
