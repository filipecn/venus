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

/// \file   vk.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-06-07
/// \brief Vulkan API access resources.

#pragma once

#include <hermes/core/types.h>
#include <venus/utils/debug.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include <volk.h>
#pragma GCC diagnostic pop

#include <vector>

namespace venus::core {

// *****************************************************************************
//                                                                  Vulkan API
// *****************************************************************************

/// Vulkan API access
class vk final {
public:
  /// Utility function for swapping handles.
  template <typename VK_HANDLE_TYPE>
  static void swap(VK_HANDLE_TYPE &a, VK_HANDLE_TYPE &b) {
    std::swap(a, b);
  }

  /// Utility function for getting data size from VkIndexType.
  static VkDeviceSize indexSize(VkIndexType type);
  /// Utility function for getting data size from VkFormat.
  static VkDeviceSize formatSize(VkFormat format);

  /// Auxiliary struct for managing version.
  class Version {
  public:
    Version(u32 major, u32 minor, u32 patch = 0) noexcept;
    Version(u32 full_version = 0) noexcept;
    ~Version() = default;

    /// \return Version number comparison.
    bool operator<(const Version &rhs) const;
    bool operator>=(const Version &rhs) const;

    /// \return The full version code in Vulkan standards.
    u32 version() const;
    /// \return The full version code in Vulkan standards.
    u32 operator*() const;

  private:
    u32 major_version_{0};
    u32 minor_version_{0};
    u32 patch_version_{0};

    /// \return A string in the format "<major version>.<minor version>"
    VENUS_TO_STRING_FRIEND(vk::Version);
  };

  /// Initializes vulkan api
  /// \param required_version [def = 0.0.0]
  /// \return Error status:
  ///          - NO_ERROR on success.
  ///          - INCOMPATIBLE_API if required version is not available.
  static VeResult init(const Version &required_version = {});

  // Layers
  // ------
  // Vulkan instance layers are optional, dynamically loaded shared libraries
  // that can intercept, evaluate, and modify Vulkan API calls. They provide
  // functionality like validation, debugging, and profiling without modifying
  // the application's source code. Common Uses of Instance Layers:
  //
  // Validation:
  //    Validation layers check for invalid API usage, helping to catch errors
  //    during development.
  //
  // Debugging:
  //    API dump layers log API calls for debugging purposes, while validation
  //    layers provide error messages. Profiling: Layers can be used to profile
  //    Vulkan application performance. Extension Support: Layers can provide
  //    implementations of extensions that are not natively supported by the
  //    driver.
  //
  // Key Considerations:
  //
  // Performance:
  //    Layers can introduce overhead, but validation layers are typically used
  //    only during development and can be disabled for production builds.
  //
  // Ordering:
  //    The order in which layers are placed in the dispatch chain can affect
  //    their behavior.
  //
  // Configuration:
  //    Vulkan layer settings can be configured using various methods, including
  //    the Vulkan API, a configuration file, or environment variables.

  /// Retrieve available validation layers.
  static const std::vector<VkLayerProperties> &availableValidationLayers();

  // Instance
  // --------
  // The Vulkan Instance holds all kind of information about the application,
  // such as application name, version, etc. The instance is the interface
  // between the application and the Vulkan Library, that can perform
  // operations like the enumeration of available physical devices and creation
  // of logical devices.

  /// Retrieve available instance extensions.
  static const std::vector<VkExtensionProperties> &
  availableInstanceExtensions();
  /// Checks if extension is supported by the current api.
  ///\param extension_name (ex: ).
  ///\return bool true if extension is supported.
  static bool isInstanceExtensionSupported(const char *extension_name);
  /// Checks if validation layer is supported by the current api.
  ///\param layer_name (ex: ).
  ///\return bool true if layer is supported.
  static bool isValidationLayerSupported(const char *layer_name);

  // Physical Devices
  // ----------------
  // Physical devices are the hardware we intend to use with Vulkan. Thus we
  // need to look for the devices that supports the features we need. We can
  // select any number of graphics cards and use them simultaneously.

  struct DeviceFeatures {
    VkPhysicalDeviceFeatures f{};
    VkPhysicalDeviceFeatures2 f2{};
    VkPhysicalDeviceVulkan13Features v13_f{};
    VkPhysicalDeviceVulkan12Features v12_f{};
    VkPhysicalDeviceDescriptorIndexingFeaturesEXT descriptor_indexing_f{};
    VkPhysicalDeviceSynchronization2FeaturesKHR synchronization2_f{};
  };

  /// Retrieve available queue families exposed by a physical device
  /// \param[in] vk_physical_device
  /// \param[out] queue_families receives the list of available queue_families.
  /// \return error status.
  static VeResult checkAvailableQueueFamilies(
      VkPhysicalDevice vk_physical_device,
      std::vector<VkQueueFamilyProperties> &queue_families);
  /// Gets the list of the properties of supported extensions for the device.
  /// \param[in] vk_physical_device
  /// \param[out] extensions receives the list of available extensions
  /// \return error status.
  static VeResult
  checkAvailableExtensions(VkPhysicalDevice vk_physical_device,
                           std::vector<VkExtensionProperties> &extensions);

  // Family Queues
  // -------------
  // Every Vulkan operation requires commands that are submitted to a queue.
  // Different queues can be processed independently and may support different
  // types of operations.
  // Queues with the same capabilities are grouped into families. A device may
  // expose any number of queue families. For example, there could be a queue
  // family that only allows processing of compute commands or one that only
  // allows memory transfer related commands.

  struct QueueFamilyConfig {
    u32 index;
    std::vector<f32> priorities;
    VkDeviceQueueCreateFlags flags;
  };

  struct GraphicsQueueFamilyIndices {
    u32 graphics_queue_family_index;
    u32 present_queue_family_index;
  };

  ~vk() = default;
  vk(const vk &) noexcept = delete;
  vk(vk &&) noexcept = delete;
  vk &operator=(const vk &) = delete;
  vk &operator=(vk &&) = delete;

private:
  static vk &get();

  vk() = default;

  // cache
  std::vector<VkExtensionProperties> vk_extensions_;
  std::vector<VkLayerProperties> vk_validation_layers_;
};

} // namespace venus::core

namespace venus {
HERMES_DECLARE_TO_STRING_DEBUG_METHOD(venus::core::vk::Version)
HERMES_DECLARE_TO_STRING_DEBUG_METHOD(
    venus::core::vk::GraphicsQueueFamilyIndices)
} // namespace venus
