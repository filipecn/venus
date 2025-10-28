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

#include <venus/scene/materials.h>
#include <venus/utils/vk_debug.h>

#include <backends/imgui_impl_vulkan.h>
#include <hermes/colors/argb_colors.h>

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

#define CREATE_SHADER_MODULE(NAME, FILE)                                       \
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(NAME,                                      \
                                    pipeline::ShaderModule::Config()           \
                                        .setEntryFuncName("main")              \
                                        .fromSpvFile(shaders_path / #FILE)     \
                                        .create(vk_device));

  // shaders
  std::filesystem::path shaders_path(VENUS_SHADERS_PATH);

  CREATE_SHADER_MODULE(vert_mesh, mesh.vert.spv)
  CREATE_SHADER_MODULE(frag_mesh_pbr, mesh_pbr.frag.spv)
  CREATE_SHADER_MODULE(vert_vdb_volume, ve_vdb_volume.vert.spv)
  CREATE_SHADER_MODULE(frag_vdb_volume, ve_vdb_volume.frag.spv)

  CREATE_SHADER_MODULE(vert_test, test.vert.spv)
  CREATE_SHADER_MODULE(vert_bindless_test, bindless_test.vert.spv)

  CREATE_SHADER_MODULE(frag_flat_color, flat_color.frag.spv)

  return VeResult::noError();

#undef CREATE_SHADER_MODULE
}

void GraphicsEngine::Globals::Shaders::clear() {
  vert_vdb_volume.destroy();
  frag_vdb_volume.destroy();

  vert_mesh.destroy();
  frag_mesh_pbr.destroy();

  vert_test.destroy();
  vert_bindless_test.destroy();

  frag_flat_color.destroy();
}

VeResult GraphicsEngine::Globals::Materials::init(GraphicsDevice &gd) {
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      color, scene::materials::Material_Test::material(gd));
#ifdef VENUS_INCLUDE_GLTF
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      gltf_metallic_roughness,
      scene::materials::GLTF_MetallicRoughness::material(gd));
#endif
#ifdef VENUS_INCLUDE_VDB
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(vdb,
                                    scene::materials::VDB_Volume::material(gd));
#endif
  return VeResult::noError();
}

void GraphicsEngine::Globals::Materials::clear() {
#ifdef VENUS_INCLUDE_GLTF
  gltf_metallic_roughness.destroy();
#endif
#ifdef VENUS_INCLUDE_VDB
  vdb.destroy();
#endif
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

    VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
        scene_data_layout_,
        pipeline::DescriptorSet::Layout::Config()
            .addLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
                              VK_SHADER_STAGE_VERTEX_BIT |
                                  VK_SHADER_STAGE_FRAGMENT_BIT)
            .addLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10,
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

VeResult GraphicsEngine::Globals::Defaults::init(GraphicsDevice &gd) {
  VkExtent2D image_size{16, 16};
  { // error image
    u32 black = hermes::colors::argb::white;
    u32 magenta = hermes::colors::argb::red_200;
    std::array<uint32_t, 16 * 16> pixels; // for 16x16 checkerboard texture
    for (int x = 0; x < 16; x++) {
      for (int y = 0; y < 16; y++) {
        pixels[y * 16 + x] = ((x % 2) ^ (y % 2)) ? magenta : black;
      }
    }
    VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
        error_image_,
        mem::AllocatedImage::Config()
            .setImageConfig(mem::Image::Config::defaults(image_size)
                                .addUsage(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                                .addUsage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT))
            .setMemoryConfig(
                mem::DeviceMemory::Config().setDeviceLocal().setUsage(
                    VMA_MEMORY_USAGE_GPU_ONLY))
            .create(*gd));

    VENUS_RETURN_BAD_RESULT(
        pipeline::ImageWritter()
            .addImage(*error_image_, pixels.data(), image_size)
            .immediateSubmit(gd));

    VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
        error_image_view_,
        mem::Image::View::Config()
            .setViewType(VK_IMAGE_VIEW_TYPE_2D)
            .setFormat(error_image_.format())
            .setSubresourceRange({VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1})
            .create(error_image_));

    error_image.image = *error_image_;
    error_image.view = *error_image_view_;
  }

  { // linear sampler
    VENUS_ASSIGN_OR_RETURN_BAD_RESULT(linear_sampler_,
                                      scene::Sampler::Config::defaults()
                                          .setMagFilter(VK_FILTER_LINEAR)
                                          .setMinFilter(VK_FILTER_LINEAR)
                                          .create(**gd));
    linear_sampler = *linear_sampler_;
  }

  { // nearest sampler
    VENUS_ASSIGN_OR_RETURN_BAD_RESULT(nearest_sampler_,
                                      scene::Sampler::Config::defaults()
                                          .setMagFilter(VK_FILTER_NEAREST)
                                          .setMinFilter(VK_FILTER_NEAREST)
                                          .create(**gd));
    nearest_sampler = *nearest_sampler_;
  }

  HERMES_UNUSED_VARIABLE(gd);
  return VeResult::noError();
}

void GraphicsEngine::Globals::Defaults::clear() {
  error_image_view_.destroy();
  error_image_.destroy();
  linear_sampler_.destroy();
  nearest_sampler_.destroy();
}

void GraphicsEngine::Globals::UI::newFrame() {
  ImGui_ImplVulkan_NewFrame();
  GraphicsEngine::display()->newUIFrame();
  ImGui::NewFrame();
}

void GraphicsEngine::Globals::UI::draw() {
  ImGuiIO &io = ImGui::GetIO();
  HERMES_UNUSED_VARIABLE(io);
  ImGui::Render();

  auto &gd = GraphicsEngine::device();

  auto rendering_info =
      pipeline::CommandBuffer::RenderingInfo()
          .setLayerCount(1)
          .setRenderArea({VkOffset2D{0, 0}, gd.swapchain().imageExtent()})
          .addColorAttachment(
              pipeline::CommandBuffer::RenderingInfo::Attachment()
                  .setImageLayout(VK_IMAGE_LAYOUT_GENERAL)
                  .setImageView(
                      *gd.swapchain().imageViews()[gd.currentTargetIndex()])
                  .setStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
                  .setLoadOp(VK_ATTACHMENT_LOAD_OP_LOAD));

  auto &cb = gd.commandBuffer();
  cb.beginRendering(*rendering_info);
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *cb);
  cb.endRendering();
}

void GraphicsEngine::Globals::UI::resize() {
  // auto extent = GraphicsEngine::display()->size();
  ImGui_ImplVulkan_SetMinImageCount(3);
}

VeResult GraphicsEngine::Globals::UI::init(engine::GraphicsEngine &ge) {

  VkDescriptorPoolSize pool_sizes[] = {
      {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
      {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
      {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  pool_info.maxSets = 1000;
  pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
  pool_info.pPoolSizes = pool_sizes;

  vk_device_ = **ge.device();
  vk_instance_ = *ge.instance_;
  vk_physical_device_ = *((*ge.device()).physical());
  vk_graphics_queue_ = ge.device().graphicsQueue();

  VENUS_VK_RETURN_BAD_RESULT(vkCreateDescriptorPool(
      vk_device_, &pool_info, nullptr, &vk_descriptor_pool_));

  // this initializes the core structures of imgui
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;

  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();

  // Setup scaling
  // ImGuiStyle &style = ImGui::GetStyle();
  // style.ScaleAllSizes(11); // Bake a fixed style scale. (until we have a
  // solution for dynamic style scaling, changing this
  // requires resetting Style + calling this again)
  // style.FontScaleDpi =
  //     11; // Set initial font scale. (using io.ConfigDpiScaleFonts=true
  //  makes this unnecessary. We leave both here for
  //  documentation purpose)

  // this initializes imgui for SDL
  HERMES_ASSERT(ge.display_);
  VENUS_RETURN_BAD_RESULT(ge.display_->initUI());

  // this initializes imgui for Vulkan
  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = vk_instance_;
  init_info.PhysicalDevice = vk_physical_device_;
  init_info.Device = vk_device_;
  init_info.Queue = vk_graphics_queue_;
  init_info.DescriptorPool = vk_descriptor_pool_;
  init_info.MinImageCount = 3;
  init_info.ImageCount = 3;
  init_info.UseDynamicRendering = true;

  // dynamic rendering parameters for imgui to use
  init_info.PipelineInfoMain.PipelineRenderingCreateInfo = {};
  init_info.PipelineInfoMain.PipelineRenderingCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
  init_info.PipelineInfoMain.PipelineRenderingCreateInfo.pNext = nullptr;
  init_info.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount =
      1;
  auto swapchain_format = ge.device().swapchain().colorFormat();
  init_info.PipelineInfoMain.PipelineRenderingCreateInfo
      .pColorAttachmentFormats = &swapchain_format;

  init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  ImGui_ImplVulkan_LoadFunctions(
      ge.instance_.apiVersion().version(),
      [](const char *function_name, void *globals_ui) {
        PFN_vkVoidFunction instanceAddr = vkGetInstanceProcAddr(
            (*(reinterpret_cast<GraphicsEngine::Globals::UI *>(globals_ui)))
                .vk_instance_,
            function_name);
        PFN_vkVoidFunction deviceAddr = vkGetDeviceProcAddr(
            (*(reinterpret_cast<GraphicsEngine::Globals::UI *>(globals_ui)))
                .vk_device_,
            function_name);
        return deviceAddr ? deviceAddr : instanceAddr;
      },
      this);
  ImGui_ImplVulkan_Init(&init_info);

  return VeResult::noError();
}

void GraphicsEngine::Globals::UI::clear() {
  ImGui_ImplVulkan_Shutdown();
  GraphicsEngine::display()->closeUI();
  ImGui::DestroyContext();
  vkDestroyDescriptorPool(vk_device_, vk_descriptor_pool_, nullptr);
}

VeResult GraphicsEngine::Globals::init(GraphicsEngine &ge) {
  auto &gd = ge.device();
  VENUS_RETURN_BAD_RESULT(descriptors.init(gd));
  VENUS_RETURN_BAD_RESULT(shaders.init(**gd));
  VENUS_RETURN_BAD_RESULT(materials.init(gd));
  VENUS_RETURN_BAD_RESULT(defaults.init(gd));
  VENUS_RETURN_BAD_RESULT(ui.init(ge));
  return VeResult::noError();
}

VeResult GraphicsEngine::Globals::cleanup() {
  shaders.clear();
  materials.clear();
  descriptors.clear();
  defaults.clear();
  ui.clear();
  return VeResult::noError();
}

mem::BufferPool &GraphicsEngine::Cache::buffers() { return buffers_; }

mem::ImagePool &GraphicsEngine::Cache::images() { return images_; }

scene::TextureCache &GraphicsEngine::Cache::textures() { return textures_; }

VeResult GraphicsEngine::Cache::cleanup() {
  buffers_.destroy();
  images_.destroy();
  textures_.clear();
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

GraphicsEngine::Config &GraphicsEngine::Config::enableUI() {
  enable_ui_ = true;
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
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
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
  s_instance.display_ = display;
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      s_instance.surface_, display->createSurface(*s_instance.instance_));

  // graphics device
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(s_instance.gd_,
                                    engine::GraphicsDevice::Config()
                                        .setSurface(*s_instance.surface_)
                                        .setSurfaceExtent(display->resolution())
                                        .setFeatures(device_features_)
                                        .addExtensions(device_extensions_)
                                        .create(s_instance.instance_));

  // init ui

  return VeResult::noError();
}

VeResult GraphicsEngine::startup() {

  // globals
  VENUS_RETURN_BAD_RESULT(s_instance.globals_.init(s_instance));

  return VeResult::noError();
}

VeResult GraphicsEngine::shutdown() {
  VENUS_RETURN_BAD_RESULT(s_instance.cache_.cleanup());
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

const io::Display *GraphicsEngine::display() { return s_instance.display_; }

} // namespace venus::engine
