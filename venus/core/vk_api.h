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
#include <venus/core/debug.h>

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
  /// \brief Auxiliary struct for managing version.
  class Version {
  public:
    Version(u32 major, u32 minor, u32 patch = 0) noexcept;
    Version(u32 full_version = 0) noexcept;
    ~Version() = default;

    /// \return Version number comparison.
    bool operator<(const Version &rhs) const;

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

  // Instance
  // --------
  // The Vulkan Instance holds all kind of information about the application,
  // such as application name, version, etc. The instance is the interface
  // between the application and the Vulkan Library, that can perform
  // operations like the enumeration of available physical devices and creation
  // of logical devices.
  //

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
HERMES_DECLARE_TO_STRING_DEBUG_METHOD(venus::core::vk::Version);
} // namespace venus
