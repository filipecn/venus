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
///\file instance.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2025-06-07
///
///\brief

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <venus/core/physical_device.h>
#include <venus/core/vk.h>

namespace venus::core {

///\brief Vulkan Instance object handle
/// The Vulkan Instance holds all kinds of information about the application,
/// such as application name, version, etc. The instance is the interface
/// between the application and the Vulkan Library.
/// \note This class uses RAII
class Instance final {
public:
  using Ptr = std::shared_ptr<Instance>;
  // ***************************************************************************
  //                                                               CONSTRUCTORS
  // ***************************************************************************
  Instance();
  ///\brief Construct a new Instance object.
  ///\param application_name **[in]**
  ///\param desired_instance_extensions **[in | default = {}]**
  explicit Instance(
      const std::string &application_name,
      const std::vector<const char *> &desired_instance_extensions =
          std::vector<const char *>(),
      const std::vector<const char *> &validation_layers =
          std::vector<const char *>());
  Instance(Instance &&other) noexcept;
  Instance(const Instance &other) = delete;
  ~Instance();
  // ***************************************************************************
  //                                                                 OPERATORS
  // ***************************************************************************
  Instance &operator=(const Instance &other) = delete;
  Instance &operator=(Instance &&other) noexcept;
  // ***************************************************************************
  //                                                                  CREATION
  // ***************************************************************************
  /// Creates the instance object
  /// \note Destroys the instance handle first if necessary.
  /// \param application_name
  /// \param desired_instance_extensions
  /// \param validation_layers
  /// \return bool if good()
  [[nodiscard]] bool
  init(const std::string &application_name,
       const std::vector<const char *> &desired_instance_extensions =
           std::vector<const char *>(),
       const std::vector<const char *> &validation_layers =
           std::vector<const char *>());
  /// Destroy and invalidate this instance.
  void destroy();
  // ***************************************************************************
  //                                                                   METHODS
  // ***************************************************************************
  ///\return bool true if instance is valid.
  [[nodiscard]] bool good() const;
  ///\note Uses vkEnumeratePhysicalDevices
  ///\param physical_devices **[out]** receives list of physical devices
  ///\return bool true if success
  [[nodiscard]] std::vector<PhysicalDevice>
  enumerateAvailablePhysicalDevices() const;
  // ***************************************************************************
  //                                                                    FIELDS
  // ***************************************************************************
  /// \return Vulkan handle
  [[nodiscard]] VkInstance handle() const;

private:
  VkInstance vk_instance_{VK_NULL_HANDLE};
  VkDebugUtilsMessengerEXT vk_debug_messenger_{VK_NULL_HANDLE};
};

} // namespace venus::core
