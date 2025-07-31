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

/// \file   instance.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-06-07

#include <venus/core/instance.h>

#include <venus/core/vk_debug.h>

#include <utility>
#include <vulkan/vulkan_core.h>

namespace venus::core {

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, debugMessenger, pAllocator);
  }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
              void *pUserData) {
  HERMES_DEBUG("validation layer: ", pCallbackData->pMessage);
  return VK_FALSE;
}

void populateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT &create_info) {
  create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  create_info.messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  create_info.pfnUserCallback = debugCallback;
}

Instance::Config &Instance::Config::setApiVersion(const vk::Version &version) {
  api_version_ = version;
  return *this;
}

Instance::Config &Instance::Config::setAppVersion(const vk::Version &version) {
  app_version_ = version;
  return *this;
}

Instance::Config &
Instance::Config::setEngineVersion(const vk::Version &version) {
  engine_version_ = version;
  return *this;
}

Instance::Config &Instance::Config::setName(const std::string_view &app_name) {
  app_name_ = app_name;
  return *this;
}

Instance::Config &
Instance::Config::setEngineName(const std::string_view &engine_name) {
  engine_name_ = engine_name;
  return *this;
}

Instance::Config &
Instance::Config::addLayer(const std::string_view &layer_name) {
  layers_.emplace_back(layer_name);
  return *this;
}

Instance::Config &
Instance::Config::addExtension(const std::string_view &extension_name) {
  extensions_.emplace_back(extension_name);
  return *this;
}

#ifdef VENUS_DEBUG

Instance::Config &Instance::Config::enableDefaultDebugMessageSeverityFlags() {
  message_severity_flags_ |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
  message_severity_flags_ |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  return *this;
}

Instance::Config &Instance::Config::enableDefaultDebugMessageTypeFlags() {
  message_type_flags_ |= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
  message_type_flags_ |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  message_type_flags_ |= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
  return *this;
}

Instance::Config &Instance::Config::addDebugMessageSeverityFlags(
    VkDebugUtilsMessageSeverityFlagsEXT flags) {
  message_severity_flags_ |= flags;
  return *this;
}

Instance::Config &Instance::Config::addDebugMessageTypeFlags(
    VkDebugUtilsMessageTypeFlagsEXT flags) {
  message_type_flags_ |= flags;
  return *this;
}

Instance::Config &Instance::Config::enableDebugUtilsExtension() {
  extensions_.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  return *this;
}

#endif

Result<Instance> Instance::Config::create() {

  std::vector<const char *> instance_extensions;
  for (auto &extension : extensions_) {
    instance_extensions.emplace_back(extension.c_str());
    if (!vk::isInstanceExtensionSupported(extension.c_str())) {
      HERMES_WARN("{}{}{}", "Extension named '", extension,
                  "' is not supported.");
      return VeResult::incompatible();
    }
  }

  std::vector<const char *> validation_layers;
  for (auto &layer : layers_) {
    validation_layers.emplace_back(layer.c_str());
    if (!vk::isValidationLayerSupported(layer.c_str())) {
      HERMES_WARN("Validation layer named '", layer, "' is not supported.");
      return Result<Instance>::error(VeResult::incompatible());
    }
  }

  VkApplicationInfo info;
  info.pApplicationName = app_name_.c_str();
  info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  info.apiVersion = *api_version_;
  info.engineVersion = *engine_version_;
  info.applicationVersion = *app_version_;
  info.pNext = nullptr;
  info.pEngineName = engine_name_.c_str();

  VkInstanceCreateInfo create_info;
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pNext = nullptr;
  create_info.flags = flags_;
  create_info.pApplicationInfo = &info;
  create_info.enabledLayerCount = validation_layers.size();
  create_info.ppEnabledLayerNames =
      (validation_layers.size()) ? validation_layers.data() : nullptr;
  create_info.enabledExtensionCount =
      static_cast<u32>(instance_extensions.size());
  create_info.ppEnabledExtensionNames =
      (instance_extensions.size()) ? instance_extensions.data() : nullptr;

#ifdef VENUS_DEBUG
  VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
  populateDebugMessengerCreateInfo(debug_create_info);
  create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debug_create_info;
#endif

  VkInstance vk_instance{VK_NULL_HANDLE};

  VENUS_VK_RETURN_BAD_RESULT(
      vkCreateInstance(&create_info, nullptr, &vk_instance));
  // load vulkan entrypoints
  volkLoadInstance(vk_instance);

#ifdef VENUS_DEBUG
  VkDebugUtilsMessengerEXT vk_debug_messenger{VK_NULL_HANDLE};
  VkDebugUtilsMessengerCreateInfoEXT debug_after_create_info;
  populateDebugMessengerCreateInfo(debug_after_create_info);
  VENUS_VK_RETURN_BAD_RESULT(CreateDebugUtilsMessengerEXT(
      vk_instance, &debug_after_create_info, nullptr, &vk_debug_messenger));
#endif

  Instance instance;
  instance.vk_instance_ = vk_instance;
#ifdef VENUS_DEBUG
  instance.vk_debug_messenger_ = vk_debug_messenger;
  instance.config_ = *this;
#endif

  return Result<Instance>(std::move(instance));
}

Instance::Instance(Instance &&rhs) noexcept { *this = std::move(rhs); }

Instance::~Instance() { destroy(); }

Instance &Instance::operator=(Instance &&rhs) noexcept {
  destroy();
  vk_instance_ = rhs.vk_instance_;
  rhs.vk_instance_ = VK_NULL_HANDLE;
#ifdef VENUS_DEBUG
  vk_debug_messenger_ = rhs.vk_debug_messenger_;
  rhs.vk_debug_messenger_ = VK_NULL_HANDLE;
  config_ = rhs.config_;
#endif
  return *this;
}

void Instance::destroy() {
  if (!vk_instance_)
    return;
#ifdef VENUS_DEBUG
  if (vk_debug_messenger_) {
    DestroyDebugUtilsMessengerEXT(vk_instance_, vk_debug_messenger_, nullptr);
    vk_debug_messenger_ = VK_NULL_HANDLE;
  }
#endif
  vkDestroyInstance(vk_instance_, nullptr);
  vk_instance_ = VK_NULL_HANDLE;
}

VkInstance Instance::operator*() const { return vk_instance_; }

Instance::operator bool() const { return vk_instance_ != VK_NULL_HANDLE; }

Result<PhysicalDevices> Instance::physicalDevices() const {
  PhysicalDevices physical_devices;
  u32 devices_count = 0;
  VENUS_VK_RETURN_BAD_RESULT(
      vkEnumeratePhysicalDevices(vk_instance_, &devices_count, nullptr));
  if (devices_count == 0) {
    HERMES_ERROR("Could not get the number of available physical devices.");
    return VeResult::notFound();
  }
  std::vector<VkPhysicalDevice> devices(devices_count);
  VENUS_VK_RETURN_BAD_RESULT(
      vkEnumeratePhysicalDevices(vk_instance_, &devices_count, devices.data()));
  if (devices_count == 0) {
    HERMES_ERROR("Could not enumerate physical devices.");
    return VeResult::notFound();
  }
  for (auto &device : devices)
    physical_devices.emplace_back(device);

  return Result<PhysicalDevices>(physical_devices);
}

HERMES_NODISCARD Result<PhysicalDevice>
Instance::findPhysicalDevice(const VkSurfaceKHR &surface,
                             vk::GraphicsQueueIndices &graphics_queues) const {}

} // namespace venus::core

namespace venus {
HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::core::Instance::Config)
HERMES_PUSH_DEBUG_VENUS_FIELD(api_version_)
HERMES_PUSH_DEBUG_VENUS_FIELD(app_version_)
HERMES_PUSH_DEBUG_VENUS_FIELD(engine_version_)
HERMES_PUSH_DEBUG_FIELD(app_name_)
HERMES_PUSH_DEBUG_FIELD(engine_name_)
HERMES_PUSH_DEBUG_ARRAY_FIELD_BEGIN(layers_, layer)
HERMES_PUSH_DEBUG_FIELD_VALUE(layer, layer)
HERMES_PUSH_DEBUG_ARRAY_FIELD_END
HERMES_PUSH_DEBUG_ARRAY_FIELD_BEGIN(extensions_, extension)
HERMES_PUSH_DEBUG_FIELD_VALUE(extension, extension)
HERMES_PUSH_DEBUG_ARRAY_FIELD_END
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::core::Instance)
HERMES_PUSH_DEBUG_VK_HANDLE(vk_instance_);
HERMES_PUSH_DEBUG_VENUS_FIELD(config_);
HERMES_TO_STRING_DEBUG_METHOD_END
} // namespace venus
