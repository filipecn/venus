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

/// \file   vk_api.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-06-07

#include <venus/core/vk_api.h>

#include <venus/core/vk_debug.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#pragma GCC diagnostic pop

namespace venus::core {

bool checkAvailableExtensions(std::vector<VkExtensionProperties> &extensions) {
  u32 extensions_count = 0;
  VENUS_VK_RETURN_BAD(vkEnumerateInstanceExtensionProperties(
                          nullptr, &extensions_count, nullptr),
                      false)
  HERMES_ASSERT(extensions_count != 0);
  extensions.resize(extensions_count);
  VENUS_VK_RETURN_BAD(vkEnumerateInstanceExtensionProperties(
                          nullptr, &extensions_count, &extensions[0]),
                      false)
  HERMES_ASSERT(extensions_count != 0);
  return true;
}

bool checkAvailableValidationLayers(
    std::vector<VkLayerProperties> &validation_layers) {
  u32 layer_count = 0;
  VENUS_VK_RETURN_BAD(vkEnumerateInstanceLayerProperties(&layer_count, nullptr),
                      false)
  HERMES_ASSERT(layer_count != 0)
  validation_layers.resize(layer_count);
  VENUS_VK_RETURN_BAD(vkEnumerateInstanceLayerProperties(
                          &layer_count, validation_layers.data()),
                      false)
  HERMES_ASSERT(layer_count != 0)
  return true;
}

bool vk::isInstanceExtensionSupported(const char *desired_instance_extension) {
  auto &vki = get();
  static bool available_loaded = false;
  if (!available_loaded) {
    HERMES_ASSERT(::venus::core::checkAvailableExtensions(vki.vk_extensions_))
    available_loaded = true;
  }

  for (const auto &extension : vki.vk_extensions_)
    if (std::string_view(extension.extensionName) ==
        std::string_view(desired_instance_extension))
      return true;
  return false;
}

bool vk::isValidationLayerSupported(const char *validation_layer) {
  auto &vki = get();
  static bool available_loaded = false;
  if (!available_loaded) {
    HERMES_ASSERT(::venus::core::checkAvailableValidationLayers(
        vki.vk_validation_layers_))
    available_loaded = true;
  }

  for (const auto &layer : vki.vk_validation_layers_)
    if (std::string_view(layer.layerName) == std::string_view(validation_layer))
      return true;
  return false;
}

VeResult vk::checkAvailableQueueFamilies(
    VkPhysicalDevice vk_physical_device,
    std::vector<VkQueueFamilyProperties> &queue_families) {
  u32 queue_families_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device,
                                           &queue_families_count, nullptr);
  if (queue_families_count == 0) {
    HERMES_ERROR("Could not get the number of queue families.\n");
    return VeResult::notFound();
  }
  queue_families.resize(queue_families_count);
  vkGetPhysicalDeviceQueueFamilyProperties(
      vk_physical_device, &queue_families_count, queue_families.data());
  if (queue_families_count == 0) {
    HERMES_ERROR("Could not acquire properties of queue families.\n");
    return VeResult::notFound();
  }
  return VeResult::noError();
}

VeResult
vk::checkAvailableExtensions(VkPhysicalDevice vk_physical_device,
                             std::vector<VkExtensionProperties> &extensions) {
  u32 extensions_count = 0;
  VENUS_VK_RETURN_BAD_RESULT(vkEnumerateDeviceExtensionProperties(
      vk_physical_device, nullptr, &extensions_count, nullptr));
  if (extensions_count == 0) {
    HERMES_ERROR("Could not get the number of queue families.\n");
    return VeResult::notFound();
  }
  extensions.resize(extensions_count);
  VENUS_VK_RETURN_BAD_RESULT(vkEnumerateDeviceExtensionProperties(
      vk_physical_device, nullptr, &extensions_count, extensions.data()));
  return VeResult::noError();
}

vk::Version::Version(u32 major, u32 minor, u32 patch) noexcept
    : major_version_(major), minor_version_(minor), patch_version_(patch) {}

vk::Version::Version(u32 version) noexcept
    : major_version_(VK_VERSION_MAJOR(version)),
      minor_version_(VK_VERSION_MINOR(version)),
      patch_version_(VK_VERSION_PATCH(version)) {}

u32 vk::Version::version() const {
  return VK_MAKE_VERSION(major_version_, minor_version_, patch_version_);
}

u32 vk::Version::operator*() const { return version(); }

bool vk::Version::operator<(const vk::Version &rhs) const {
  if (major_version_ == rhs.major_version_) {
    if (minor_version_ == rhs.minor_version_) {
      return patch_version_ < rhs.patch_version_;
    }
    return minor_version_ < rhs.minor_version_;
  }
  return major_version_ < rhs.major_version_;
}

void sanityCheck() {
  void *ptr;

  // This won't compile if the appropriate Vulkan platform define isn't set.
  ptr =
#if defined(_WIN32)
      &vkCreateWin32SurfaceKHR;
#elif defined(__linux__) || defined(__unix__)
      &vkCreateXlibSurfaceKHR;
#elif defined(__APPLE__)
      &vkCreateMacOSSurfaceMVK;
#else
      NULL;
#endif

  HERMES_UNUSED_VARIABLE(ptr);
}

vk &vk::get() {
  static vk s_vk;
  return s_vk;
}

VeResult vk::init(const vk::Version &required_version) {
  sanityCheck();

  VENUS_VK_RETURN_BAD_RESULT(volkInitialize());

  auto version = vk::Version(volkGetInstanceVersion());
  HERMES_INFO("Detected Vulkan version: {}", venus::to_string(version));
  if (version < required_version) {
    HERMES_ERROR(
        "Available Vulkan version ({}) incompatible to required version ({})",
        venus::to_string(version), venus::to_string(required_version));
    return VeResult::incompatible();
  }

  auto &v = get();
  return VeResult::noError();
}

} // namespace venus::core

namespace venus {
#ifdef VENUS_INCLUDE_TO_STRING
HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::core::vk::Version)
return std::format("{}.{}.{}", object.major_version_, object.minor_version_,
                   object.patch_version_);
HERMES_TO_STRING_DEBUG_METHOD_END
#endif
} // namespace venus
