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

#include <venus/utils/vk_debug.h>

#include <utility>
#include <vulkan/vulkan_core.h>

namespace venus::core {

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *p_create_info,
    const VkAllocationCallbacks *p_allocator,
    VkDebugUtilsMessengerEXT *p_debug_messenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, p_create_info, p_allocator, p_debug_messenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debug_messenger,
                                   const VkAllocationCallbacks *p_allocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, debug_messenger, p_allocator);
  }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT /*message_severity*/,
              VkDebugUtilsMessageTypeFlagsEXT /*message_type*/,
              const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
              void * /*p_user_data*/) {

#ifdef VENUS_NO_DEBUG
  switch (static_cast<uint32_t>(callback_data->messageIdNumber)) {
  case 0:
    // Validation Warning: Override layer has override paths set to
    // C:/VulkanSDK/<version>/Bin
    return vk::False;
  case 0x822806fa:
    // Validation Warning: vkCreateInstance(): to enable extension
    // VK_EXT_debug_utils, but this extension is intended to support use by
    // applications when debugging and it is strongly recommended that it be
    // otherwise avoided.
    return vk::False;
  case 0xe8d1a9fe:
    // Validation Performance Warning: Using debug builds of the validation
    // layers *will* adversely affect performance.
    return vk::False;
  }
#endif

  // std::cerr << vk::to_string(message_severity) << ": "
  //           << vk::to_string(message_types) << ":\n";
  HERMES_ERROR("\tmessageIDName   = <{}>", callback_data->pMessageIdName);
  HERMES_ERROR("\tmessageIdNumber = {}", callback_data->messageIdNumber);
  HERMES_ERROR("\tmessage         = <{}>", callback_data->pMessage);
  if (0 < callback_data->queueLabelCount) {
    HERMES_ERROR("\tQueue Labels:");
    for (uint32_t i = 0; i < callback_data->queueLabelCount; i++) {
      HERMES_ERROR("\t\tlabelName = <{}>",
                   callback_data->pQueueLabels[i].pLabelName);
    }
  }
  if (0 < callback_data->cmdBufLabelCount) {
    HERMES_ERROR("\tCommandBuffer Labels:");
    for (uint32_t i = 0; i < callback_data->cmdBufLabelCount; i++) {
      HERMES_ERROR("\t\tlabelName = <{}>",
                   callback_data->pCmdBufLabels[i].pLabelName);
    }
  }
  if (0 < callback_data->objectCount) {
    HERMES_ERROR("\tObjects:");
    for (uint32_t i = 0; i < callback_data->objectCount; i++) {
      HERMES_ERROR("\t\tObject {}", i);
      HERMES_ERROR("\t\t\tobjectType   = {}",
                   string_VkObjectType(callback_data->pObjects[i].objectType));
      HERMES_ERROR("\t\t\tobjectHandle = {}",
                   callback_data->pObjects[i].objectHandle);
      if (callback_data->pObjects[i].pObjectName) {
        HERMES_ERROR("\t\t\tobjectName   = <{}>",
                     callback_data->pObjects[i].pObjectName);
      }
    }
  }

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

Instance::Config &Instance::Config::addExtensions(
    const std::vector<std::string> &extension_names) {
  extensions_.insert(extensions_.end(), extension_names.begin(),
                     extension_names.end());
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

  u32 vk_version{VK_API_VERSION_1_0};
  vkEnumerateInstanceVersion(&vk_version);
  vk::Version max_version(vk_version);
  if (max_version < api_version_) {
    HERMES_ERROR("Incompatible Instance version {} (available {}).",
                 venus::to_string(api_version_), venus::to_string(max_version));
    return VeResult::incompatible();
  }

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
  instance.version_ = api_version_;
#ifdef VENUS_DEBUG
  instance.vk_debug_messenger_ = vk_debug_messenger;
  instance.config_ = *this;
#endif

  return Result<Instance>(std::move(instance));
}

Instance::Instance(Instance &&rhs) noexcept { *this = std::move(rhs); }

Instance::~Instance() noexcept { destroy(); }

Instance &Instance::operator=(Instance &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void Instance::swap(Instance &rhs) noexcept {
  VENUS_SWAP_FIELD_WITH_RHS(vk_instance_);
#ifdef VENUS_DEBUG
  VENUS_SWAP_FIELD_WITH_RHS(vk_debug_messenger_);
  VENUS_SWAP_FIELD_WITH_RHS(config_);
#endif
}

void Instance::destroy() noexcept {
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
  for (auto &device : devices) {
    PhysicalDevice pd(device, vk_instance_);
    if (vk::Version(pd.properties().apiVersion) >= version_)
      physical_devices.emplace_back(std::move(pd));
  }

  return Result<PhysicalDevices>(physical_devices);
}

} // namespace venus::core

namespace venus {
HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::core::Instance::Config)
HERMES_PUSH_DEBUG_TITLE
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
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_VK_HANDLE(vk_instance_);
HERMES_PUSH_DEBUG_VENUS_FIELD(config_);
HERMES_TO_STRING_DEBUG_METHOD_END
} // namespace venus
