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

/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-06-07
/// \brief  Example on how to initialize venus.

#include <venus/core/instance.h>
#include <venus/io/glfw_display.h>
#include <venus/io/swapchain.h>

std::vector<std::string> getInstanceExtensions() {
  std::vector<std::string> extensions;
  extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
  extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_METAL_EXT)
  extensions.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_VI_NN)
  extensions.push_back(VK_NN_VI_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
  extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
  extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
  extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
  extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
  extensions.push_back(VK_EXT_ACQUIRE_XLIB_DISPLAY_EXTENSION_NAME);
#endif
  return extensions;
}

int main() {
  using namespace venus::core;
  using namespace venus::io;
  VENUS_CHECK_VE_RESULT(vk::init());
  Instance instance;
  PhysicalDevices physical_devices;
  GLFW_Window window;
  VkSurfaceKHR surface;
  PhysicalDevice physical_device;
  vk::GraphicsQueueFamilyIndices indices;
  VENUS_ASSIGN_RESULT(instance, Instance::Config()
                                    .setApiVersion(vk::Version(1, 4, 0))
                                    .setName("hello_vulkan_app")
                                    .addExtensions(getInstanceExtensions())
                                    .enableDefaultDebugMessageSeverityFlags()
                                    .enableDefaultDebugMessageTypeFlags()
                                    .enableDebugUtilsExtension()
                                    .create());

  HERMES_INFO("\n{}", venus::to_string(instance));
  VENUS_ASSIGN_RESULT(physical_devices, instance.physicalDevices());
  HERMES_INFO("\n{}", venus::to_string(physical_devices));
  VENUS_CHECK_VE_RESULT(window.init("Hello Vulkan App", {1024, 1024}));
  VENUS_ASSIGN_RESULT(surface, window.createSurface(*instance));
  VENUS_ASSIGN_RESULT(physical_device,
                      physical_devices.select(
                          PhysicalDevices::Selector().forGraphics(surface)));
  VENUS_ASSIGN_RESULT(
      indices, physical_device.selectGraphicsQueueFamilyIndices(surface));
  HERMES_INFO("{}", venus::to_string(indices));
  Device device;
  vk::DeviceFeatures device_features;
  device_features.f.shaderUniformBufferArrayDynamicIndexing = true;
  device_features.descriptor_indexing_f.descriptorBindingPartiallyBound = true;
  device_features.synchronization2_f.synchronization2 = true;

  device_features.v13_f.dynamicRendering = true;
  device_features.v13_f.synchronization2 = true;
  device_features.v12_f.bufferDeviceAddress = true;
  device_features.v12_f.descriptorIndexing = true;
  device_features.v12_f.descriptorBindingPartiallyBound = true;
  device_features.v12_f.descriptorBindingVariableDescriptorCount = true;
  device_features.v12_f.runtimeDescriptorArray = true;
  device_features.f2.features.shaderUniformBufferArrayDynamicIndexing = true;

  std::vector<std::string> device_extensions = {
      VK_KHR_MAINTENANCE_1_EXTENSION_NAME,
      VK_KHR_MAINTENANCE_3_EXTENSION_NAME,
      VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
      VK_KHR_DEVICE_GROUP_EXTENSION_NAME,
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
      VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME};

  VENUS_ASSIGN_RESULT(
      device, Device::Config()
                  .setFeatures(device_features)
                  .addQueueFamily(indices.graphics_queue_family_index, {1.f})
                  .addQueueFamily(indices.present_queue_family_index, {1.f})
                  .addExtensions(device_extensions)
                  .create(physical_device));

  Swapchain swapchain;
  VENUS_ASSIGN_RESULT(swapchain, Swapchain::Config()
                                     .setSurface(surface)
                                     .setQueueFamilyIndices(indices)
                                     .setExtent(window.resolution())
                                     .create(device));
  return 0;
}
