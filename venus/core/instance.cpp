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
///\file instance.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2025-06-07
///
///\brief

#include <venus/core/debug.h>
#include <venus/core/instance.h>

namespace venus::core {

class SupportInfo {
public:
  /// Checks if extension is supported by the instance
  ///\param desired_instance_extension **[in]** extension name (ex: )
  ///\return bool true if extension is supported
  static bool
  isInstanceExtensionSupported(const char *desired_instance_extension) {
    static bool available_loaded = false;
    if (!available_loaded) {
      VENUS_ASSERT(checkAvailableExtensions(vk_extensions_))
      available_loaded = true;
    }

    for (const auto &extension : vk_extensions_)
      if (std::string(extension.extensionName) ==
          std::string(desired_instance_extension))
        return true;
    return false;
  }
  static bool isValidationLayerSupported(const char *validation_layer) {
    static bool available_loaded = false;
    if (!available_loaded) {
      VENUS_ASSERT(checkAvailableValidationLayers(vk_validation_layers_))
      available_loaded = true;
    }

    for (const auto &layer : vk_validation_layers_)
      if (std::string(layer.layerName) == std::string(validation_layer))
        return true;
    return false;
  }

private:
  /// Gets the list of the properties of supported instance extensions on the
  /// current hardware platform.
  /// \param extensions **[out]** list of extensions
  /// \return bool true if success
  static bool
  checkAvailableExtensions(std::vector<VkExtensionProperties> &extensions) {
    u32 extensions_count = 0;
    R_CHECK_VULKAN(vkEnumerateInstanceExtensionProperties(
                       nullptr, &extensions_count, nullptr),
                   false)
    VENUS_ASSERT(extensions_count != 0);
    extensions.resize(extensions_count);
    R_CHECK_VULKAN(vkEnumerateInstanceExtensionProperties(
                       nullptr, &extensions_count, &extensions[0]),
                   false)
    VENUS_ASSERT(extensions_count != 0);
    return true;
  }
  static bool checkAvailableValidationLayers(
      std::vector<VkLayerProperties> &validation_layers) {
    u32 layer_count = 0;
    R_CHECK_VULKAN(vkEnumerateInstanceLayerProperties(&layer_count, nullptr),
                   false)
    VENUS_ASSERT(layer_count != 0)
    validation_layers.resize(layer_count);
    R_CHECK_VULKAN(vkEnumerateInstanceLayerProperties(&layer_count,
                                                      validation_layers.data()),
                   false)
    VENUS_ASSERT(layer_count != 0)
    return true;
  }

  static std::vector<VkExtensionProperties> vk_extensions_;
  static std::vector<VkLayerProperties> vk_validation_layers_;
};

std::vector<VkExtensionProperties> SupportInfo::vk_extensions_;
std::vector<VkLayerProperties> SupportInfo::vk_validation_layers_;

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
  VENUS_DEBUG("validation layer: ", pCallbackData->pMessage);
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

Instance::Instance() = default;

Instance::Instance(const std::string &application_name,
                   const std::vector<const char *> &desired_instance_extensions,
                   const std::vector<const char *> &validation_layers) {
  init(application_name, desired_instance_extensions, validation_layers);
}

Instance::Instance(Instance &&other) noexcept {
  destroy();
  vk_instance_ = other.vk_instance_;
  vk_debug_messenger_ = other.vk_debug_messenger_;
  other.vk_instance_ = VK_NULL_HANDLE;
  other.vk_debug_messenger_ = VK_NULL_HANDLE;
}

Instance::~Instance() { destroy(); }

Instance &Instance::operator=(Instance &&other) noexcept {
  destroy();
  vk_instance_ = other.vk_instance_;
  vk_debug_messenger_ = other.vk_debug_messenger_;
  other.vk_instance_ = VK_NULL_HANDLE;
  other.vk_debug_messenger_ = VK_NULL_HANDLE;
  return *this;
}

bool Instance::init(
    const std::string &application_name,
    const std::vector<const char *> &desired_instance_extensions,
    const std::vector<const char *> &validation_layers) {
  destroy();
  SupportInfo support_info;
  std::vector<const char *> instance_extensions = desired_instance_extensions;
  instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  for (auto &extension : instance_extensions)
    if (!support_info.isInstanceExtensionSupported(extension)) {
      VENUS_WARN("{}{}{}", "Extension named '", extension,
                 "' is not supported.");
      return false;
    }
  for (auto &layer : validation_layers)
    if (!support_info.isValidationLayerSupported(layer)) {
      VENUS_WARN("Validation layer named '", layer, "' is not supported.");
      return false;
    }
  VkApplicationInfo info;
  info.pApplicationName = application_name.c_str();
  info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  info.apiVersion = VK_MAKE_VERSION(1, 0, 0);
  info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  info.pNext = nullptr;
  info.pEngineName = "venus";
  VkInstanceCreateInfo create_info;
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pNext = nullptr;
  create_info.flags = 0;
  create_info.pApplicationInfo = &info;
  create_info.enabledLayerCount = validation_layers.size();
  create_info.ppEnabledLayerNames =
      (validation_layers.size()) ? validation_layers.data() : nullptr;
  create_info.enabledExtensionCount =
      static_cast<u32>(instance_extensions.size());
  create_info.ppEnabledExtensionNames =
      (instance_extensions.size()) ? instance_extensions.data() : nullptr;

  VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
  populateDebugMessengerCreateInfo(debug_create_info);
  create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debug_create_info;

  R_CHECK_VULKAN(vkCreateInstance(&create_info, nullptr, &vk_instance_), false)

  VkDebugUtilsMessengerCreateInfoEXT debug_after_create_info;
  populateDebugMessengerCreateInfo(debug_after_create_info);
  R_CHECK_VULKAN(CreateDebugUtilsMessengerEXT(vk_instance_,
                                              &debug_after_create_info, nullptr,
                                              &vk_debug_messenger_),
                 false)
  return good();
}

void Instance::destroy() {
  if (good()) {
    if (vk_debug_messenger_)
      DestroyDebugUtilsMessengerEXT(vk_instance_, vk_debug_messenger_, nullptr);
    // TODO why this gives a seg fault?!
    // vkDestroyInstance(vk_instance_, nullptr);
  }
  vk_debug_messenger_ = VK_NULL_HANDLE;
  vk_instance_ = VK_NULL_HANDLE;
}

VkInstance Instance::handle() const { return vk_instance_; }

bool Instance::good() const { return vk_instance_ != VK_NULL_HANDLE; }

std::vector<PhysicalDevice>
Instance::enumerateAvailablePhysicalDevices() const {
  std::vector<PhysicalDevice> physical_devices;
  u32 devices_count = 0;
  R_CHECK_VULKAN(
      vkEnumeratePhysicalDevices(vk_instance_, &devices_count, nullptr),
      physical_devices)
  if (devices_count == 0) {
    VENUS_ERROR("Could not get the number of available physical devices.");
    return physical_devices;
  }
  std::vector<VkPhysicalDevice> devices(devices_count);
  R_CHECK_VULKAN(
      vkEnumeratePhysicalDevices(vk_instance_, &devices_count, devices.data()),
      physical_devices)
  if (devices_count == 0) {
    VENUS_ERROR("Could not enumerate physical devices.");
    return physical_devices;
  }
  for (auto &device : devices)
    physical_devices.emplace_back(device);

  return physical_devices;
}

} // namespace venus::core
