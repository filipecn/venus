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

/// \file   graphics_engine.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30

#include <venus/engine/graphics_engine.h>

#include <venus/engine/materials.h>

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

namespace venus::engine {

GraphicsEngine GraphicsEngine::s_instance;

VeResult GraphicsEngine::Globals::Shaders::init(VkDevice vk_device) {

  // shaders
  std::filesystem::path shaders_path(VENUS_SHADERS_PATH);
  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      vert_mesh, pipeline::ShaderModule::Config()
                     .setEntryFuncName("main")
                     .fromSpvFile(shaders_path / "mesh.vert.spv")
                     .create(vk_device));
  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      frag_mesh_pbr, pipeline::ShaderModule::Config()
                         .setEntryFuncName("main")
                         .fromSpvFile(shaders_path / "mesh_pbr.frag.spv")
                         .create(vk_device));

  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      vert_color, pipeline::ShaderModule::Config()
                      .setEntryFuncName("main")
                      .fromSpvFile(shaders_path / "color.vert.spv")
                      .create(vk_device));
  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      frag_color, pipeline::ShaderModule::Config()
                      .setEntryFuncName("main")
                      .fromSpvFile(shaders_path / "color.frag.spv")
                      .create(vk_device));

  return VeResult::noError();
}

void GraphicsEngine::Globals::Shaders::clear() {
  vert_mesh.destroy();
  frag_mesh_pbr.destroy();
  vert_color.destroy();
  frag_color.destroy();
}

VeResult GraphicsEngine::Globals::Materials::init(GraphicsDevice &gd) {
  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(color,
                                           scene::Material_Color::material(gd));
  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      gltf_metallic_roughness, scene::GLTF_MetallicRoughness::material(gd));
  return VeResult::noError();
}

void GraphicsEngine::Globals::Materials::clear() {
  gltf_metallic_roughness.destroy();
  color.destroy();
}

VeResult GraphicsEngine::Globals::Descriptors::init(GraphicsDevice &gd) {
  { // scene_data_layout
    std::array<VkDescriptorBindingFlags, 2> binding_flags{
        // binding 0
        VkDescriptorBindingFlags{},
        // binding 1
        VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT};

    VkDescriptorSetLayoutBindingFlagsCreateInfo flags{};
    flags.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    flags.pNext = nullptr;
    flags.bindingCount = 2;
    flags.pBindingFlags = binding_flags.data();

    VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
        scene_data_layout_,
        pipeline::DescriptorSet::Layout::Config()
            .addLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
                              VK_SHADER_STAGE_VERTEX_BIT |
                                  VK_SHADER_STAGE_FRAGMENT_BIT)
            .addLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
                              VK_SHADER_STAGE_VERTEX_BIT |
                                  VK_SHADER_STAGE_FRAGMENT_BIT)
            .create(**gd, &flags));

    scene_data_layout = *scene_data_layout_;
  }

  return VeResult::noError();
}

void GraphicsEngine::Globals::Descriptors::clear() {
  scene_data_layout_.destroy();
  scene_data_layout = VK_NULL_HANDLE;
}

void GraphicsEngine::Globals::Defaults::clear() {}

VeResult GraphicsEngine::Globals::init(GraphicsDevice &gd) {
  VENUS_RETURN_BAD_RESULT(descriptors.init(gd));
  VENUS_RETURN_BAD_RESULT(shaders.init(**gd));
  VENUS_RETURN_BAD_RESULT(materials.init(gd));
  return VeResult::noError();
}

VeResult GraphicsEngine::Globals::cleanup() {
  shaders.clear();
  materials.clear();
  descriptors.clear();
  defaults.clear();
  return VeResult::noError();
}

GraphicsEngine::Config &GraphicsEngine::Config::setSynchronization2() {
  device_features_.synchronization2_f.synchronization2 = true;

  device_features_.v13_f.synchronization2 = true;
  return *this;
}

GraphicsEngine::Config &GraphicsEngine::Config::setBindless() {
  device_features_.f.shaderUniformBufferArrayDynamicIndexing = true;
  device_features_.descriptor_indexing_f.descriptorBindingPartiallyBound = true;

  device_features_.v12_f.bufferDeviceAddress = true;
  device_features_.v12_f.descriptorIndexing = true;
  device_features_.v12_f.descriptorBindingPartiallyBound = true;
  device_features_.v12_f.descriptorBindingVariableDescriptorCount = true;
  device_features_.v12_f.runtimeDescriptorArray = true;
  device_features_.f2.features.shaderUniformBufferArrayDynamicIndexing = true;

  device_extensions_.insert(
      device_extensions_.end(),
      {VK_KHR_MAINTENANCE_1_EXTENSION_NAME, VK_KHR_MAINTENANCE_3_EXTENSION_NAME,
       VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
       VK_KHR_DEVICE_GROUP_EXTENSION_NAME, VK_KHR_SWAPCHAIN_EXTENSION_NAME,
       VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME});

  return *this;
}

GraphicsEngine::Config &GraphicsEngine::Config::setDynamicRendering() {
  device_features_.v13_f.dynamicRendering = true;

  device_extensions_.emplace_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

  return *this;
}

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(GraphicsEngine, setDeviceFeatures,
                                     const core::vk::DeviceFeatures &,
                                     device_features_ = value);
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(GraphicsEngine, setDeviceExtensions,
                                     const std::vector<std::string> &,
                                     device_extensions_ = value);

VeResult GraphicsEngine::Config::init(const io::Display *display) const {

  // vulkan instance
  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      s_instance.instance_, core::Instance::Config()
                                .setApiVersion(core::vk::Version(1, 4, 0))
                                .setName("hello_vulkan_app")
                                .addExtensions(getInstanceExtensions())
                                .enableDefaultDebugMessageSeverityFlags()
                                .enableDefaultDebugMessageTypeFlags()
                                .enableDebugUtilsExtension()
                                .create());

  HERMES_INFO("\n{}", venus::to_string(s_instance.instance_));
  // output surface
  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      s_instance.surface_, display->createSurface(*s_instance.instance_));

  // graphics device
  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      s_instance.gd_, engine::GraphicsDevice::Config()
                          .setSurface(*s_instance.surface_)
                          .setSurfaceExtent(display->resolution())
                          .setFeatures(device_features_)
                          .addExtensions(device_extensions_)
                          .create(s_instance.instance_));

  // globals
  VENUS_RETURN_BAD_RESULT(s_instance.globals_.init(s_instance.gd_));

  return VeResult::noError();
}

VeResult GraphicsEngine::startup(const GraphicsEngine::Config &config) {
  return VeResult::noError();
}

VeResult GraphicsEngine::shutdown() {
  VENUS_RETURN_BAD_RESULT(s_instance.globals_.cleanup());
  s_instance.gd_.destroy();
  s_instance.surface_.destroy();
  s_instance.instance_.destroy();
  return VeResult::noError();
}

GraphicsEngine::Globals &GraphicsEngine::globals() {
  return s_instance.globals_;
}

GraphicsEngine::Cache &GraphicsEngine::cache() { return s_instance.cache_; }

GraphicsDevice &GraphicsEngine::device() { return s_instance.gd_; }

} // namespace venus::engine
