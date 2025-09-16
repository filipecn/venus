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

#include <venus/app/display_app.h>
#include <venus/engine/gltf_io.h>
#include <venus/engine/graphics_device.h>
#include <venus/engine/graphics_engine.h>
#include <venus/io/glfw_display.h>

VeResult startup(venus::app::DisplayApp &app) {

  venus::core::vk::DeviceFeatures device_features;
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

  VENUS_RETURN_BAD_RESULT(venus::engine::GraphicsEngine::Config()
                              .setDeviceFeatures(device_features)
                              .setDeviceExtensions(device_extensions)
                              .init(app.display()));

  VENUS_RETURN_BAD_RESULT(venus::engine::GraphicsEngine::startup());

  venus::scene::GLTF_Node::Ptr node;
  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      node, venus::scene::GLTF_Node::from(
                std::filesystem::path(VENUS_EXAMPLE_ASSETS_PATH) / "box.glb",
                venus::engine::GraphicsEngine::device()));

  return VeResult::noError();
}

VeResult shutdown() { return venus::engine::GraphicsEngine::shutdown(); }

VeResult render(const venus::io::DisplayLoop::Iteration::Frame &frame) {
  return VeResult::noError();
}

int main() {
  using namespace venus::core;
  using namespace venus::io;
  using namespace venus::app;
  VENUS_CHECK_VE_RESULT(vk::init());

  return DisplayApp::Config()
      .setDisplay<GLFW_Window>("Hello Vulkan Display App", {1024, 1024})
      .setStartupFn(startup)
      .setShutdownFn(shutdown)
      .setRenderFn(render)
      .create()
      .run();

  // Instance instance;
  // venus::engine::GraphicsDevice gd;
  // PhysicalDevices physical_devices;
  // GLFW_Window window;
  // VkSurfaceKHR surface;
  // PhysicalDevice physical_device;
  // vk::GraphicsQueueFamilyIndices indices;
  //  instance
  //  VENUS_ASSIGN_RESULT(instance, Instance::Config()
  //                                   .setApiVersion(vk::Version(1, 4, 0))
  //                                   .setName("hello_vulkan_app")
  //                                   .addExtensions(getInstanceExtensions())
  //                                   .enableDefaultDebugMessageSeverityFlags()
  //                                   .enableDefaultDebugMessageTypeFlags()
  //                                   .enableDebugUtilsExtension()
  //                                   .create());
  // HERMES_INFO("\n{}", venus::to_string(instance));

  // window
  // VENUS_CHECK_VE_RESULT(window.init("Hello Vulkan App", {1024, 1024}));
  // VENUS_ASSIGN_RESULT(surface, window.createSurface(*instance));
  // graphics device

  // VENUS_ASSIGN_RESULT(gd, venus::engine::GraphicsDevice::Config()
  //                             .setSurface(surface)
  //                             .setSurfaceExtent(window.resolution())
  //                             .addExtensions(device_extensions)
  //                             .create(instance));

  // return 0;

  // VENUS_ASSIGN_RESULT(physical_devices, instance.physicalDevices());
  // HERMES_INFO("\n{}", venus::to_string(physical_devices));
  // VENUS_ASSIGN_RESULT(physical_device,
  //                     physical_devices.select(
  //                         PhysicalDevices::Selector().forGraphics(surface)));
  // VENUS_ASSIGN_RESULT(
  //     indices, physical_device.selectGraphicsQueueFamilyIndices(surface));
  // HERMES_INFO("{}", venus::to_string(indices));

  // Device device;
  // VENUS_ASSIGN_RESULT(
  //     device, Device::Config()
  //                 .setFeatures(device_features)
  //                 .addQueueFamily(indices.graphics_queue_family_index, {1.f})
  //                 .addQueueFamily(indices.present_queue_family_index, {1.f})
  //                 .addExtensions(device_extensions)
  //                 .create(physical_device));

  // Swapchain swapchain;
  // VENUS_ASSIGN_RESULT(swapchain, Swapchain::Config()
  //                                    .setSurface(surface)
  //                                    .setQueueFamilyIndices(indices)
  //                                    .setExtent(window.resolution())
  //                                    .create(device));
  // return 0;
}
