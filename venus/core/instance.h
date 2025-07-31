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

/// \file   instance.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-06-07
/// \brief Vulkan instance holder.

#pragma once

#include <venus/core/physical_device.h>

#include <memory>
#include <string>
#include <vector>

namespace venus::core {

/// Holder of a Vulkan instance object.
/// The Vulkan Instance holds all kinds of information about the application,
/// such as application name, version, etc. The instance is the interface
/// between the application and the Vulkan Library.
/// \note This class uses RAII
class Instance final {
public:
  using Ptr = std::shared_ptr<Instance>;

  /// Builder for Instance class.
  struct Config {
    /// \param version Required Vulkan api version.
    Config &setApiVersion(const vk::Version &version);
    /// \param version Application version.
    Config &setAppVersion(const vk::Version &version);
    /// \param version Engine version.
    Config &setEngineVersion(const vk::Version &version);
    /// \param app_name
    Config &setName(const std::string_view &app_name);
    /// \param engine_name
    Config &setEngineName(const std::string_view &engine_name);
    /// \param layer_name
    Config &addLayer(const std::string_view &layer_name);
    /// \param extension_name
    Config &addExtension(const std::string_view &extension_name);
#ifdef VENUS_DEBUG
    /// Enable flags:
    ///   - VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING
    ///   - VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR
    Config &enableDefaultDebugMessageSeverityFlags();
    /// Enable flags:
    ///   - VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
    ///   - VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    ///   - VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    Config &enableDefaultDebugMessageTypeFlags();
    Config &
    addDebugMessageSeverityFlags(VkDebugUtilsMessageSeverityFlagsEXT flags);
    Config &addDebugMessageTypeFlags(VkDebugUtilsMessageTypeFlagsEXT flags);
    Config &enableDebugUtilsExtension();
#endif
    ///
    HERMES_NODISCARD Result<Instance> create();

  private:
    // create config
    VkInstanceCreateFlags flags_{};
    // API
    vk::Version api_version_{VK_API_VERSION_1_0};
    vk::Version engine_version_{VK_API_VERSION_1_0};
    vk::Version app_version_{VK_API_VERSION_1_0};
    // Instance
    std::string app_name_{};
    std::string engine_name_{"venus_engine"};
    std::vector<std::string> layers_{};
    std::vector<std::string> extensions_{};

#ifdef VENUS_DEBUG
    VkDebugUtilsMessageSeverityFlagsEXT message_severity_flags_;
    VkDebugUtilsMessageTypeFlagsEXT message_type_flags_;
#endif

    VENUS_TO_STRING_FRIEND(Instance::Config);
  };

  Instance() = default;
  Instance(Instance &&rhs) noexcept;
  Instance &operator=(Instance &&rhs) noexcept;
  ~Instance();

  // raii

  Instance(const Instance &) = delete;
  Instance &operator=(const Instance &) = delete;

  /// Destroys and invalidate this instance.
  void destroy();
  /// \return bool true if instance is valid.
  HERMES_NODISCARD operator bool() const;
  /// \return list of detected physical devices.
  HERMES_NODISCARD Result<PhysicalDevices> physicalDevices() const;
  /// \brief Detects any hardware capable of graphics and presentation.
  /// \param surface Output surface object.
  /// \param graphics_queues Graphics family queues.
  HERMES_NODISCARD Result<PhysicalDevice>
  findPhysicalDevice(const VkSurfaceKHR &surface,
                     vk::GraphicsQueueIndices &graphics_queues) const;

  /// \return Vulkan handle
  HERMES_NODISCARD VkInstance operator*() const;

private:
  VkInstance vk_instance_{VK_NULL_HANDLE};
#ifdef VENUS_DEBUG
  VkDebugUtilsMessengerEXT vk_debug_messenger_{VK_NULL_HANDLE};
  Config config_{};
#endif

  VENUS_TO_STRING_FRIEND(Instance);
};

} // namespace venus::core
