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

/// \file   ray_tracer.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-11-12

#include <venus/pipeline/ray_tracer.h>

#include <hermes/storage/memory.h>
#include <venus/utils/vk_debug.h>

namespace venus::pipeline {

RayTracer::RayTracer(RayTracer &&rhs) noexcept { *this = std::move(rhs); }

RayTracer::~RayTracer() noexcept { destroy(); }

RayTracer &RayTracer::operator=(RayTracer &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void RayTracer::destroy() noexcept {
  tlas_.destroy();
  blas_.destroy();
  image_view_.destroy();
  image_.destroy();
  descriptor_set_.destroy();
  descriptor_set_layout_.destroy();
  descriptor_allocator_.destroy();
  pipeline_.destroy();
  pipeline_layout_.destroy();
  ubo_.destroy();
  raygen_sbt_.destroy();
  miss_sbt_.destroy();
  hit_sbt_.destroy();
}

void RayTracer::swap(RayTracer &rhs) {
  VENUS_SWAP_FIELD_WITH_RHS(blas_);
  VENUS_SWAP_FIELD_WITH_RHS(tlas_);
  VENUS_SWAP_FIELD_WITH_RHS(image_);
  VENUS_SWAP_FIELD_WITH_RHS(image_view_);
  VENUS_SWAP_FIELD_WITH_RHS(pipeline_layout_);
  VENUS_SWAP_FIELD_WITH_RHS(pipeline_);
  VENUS_SWAP_FIELD_WITH_RHS(descriptor_allocator_);
  VENUS_SWAP_FIELD_WITH_RHS(descriptor_set_layout_);
  VENUS_SWAP_FIELD_WITH_RHS(descriptor_set_);
  VENUS_SWAP_FIELD_WITH_RHS(ubo_);
  VENUS_SWAP_FIELD_WITH_RHS(raygen_sbt_);
  VENUS_SWAP_FIELD_WITH_RHS(miss_sbt_);
  VENUS_SWAP_FIELD_WITH_RHS(hit_sbt_);
}

RayTracer &RayTracer::setResolution(const VkExtent2D &resolution) {
  resolution_ = resolution;
  return *this;
}

RayTracer &RayTracer::add(const RayTracer::TracerObject &tracer_object) {
  VkDeviceOrHostAddressConstKHR vertex_data_device_address{};
  VkDeviceOrHostAddressConstKHR index_data_device_address{};
  VkDeviceOrHostAddressConstKHR transform_matrix_device_address{};
  vertex_data_device_address.deviceAddress = tracer_object.vertex_data;
  index_data_device_address.deviceAddress = tracer_object.index_data;
  transform_matrix_device_address.deviceAddress = tracer_object.transform_data;

  auto triangles_data =
      scene::AccelerationStructure::TrianglesData()
          // vertex
          .setVertexData(vertex_data_device_address)
          .setVertexFormat(
              tracer_object.vertex_layout
                  .componentFormat(mem::VertexLayout::ComponentType::Position)
                  .value())
          .setVertexStride(tracer_object.vertex_layout.stride())
          .setMaxVertex(tracer_object.max_vertex)
          // transform
          .setTransformData(transform_matrix_device_address);

  if (index_data_device_address.deviceAddress) {
    // index
    triangles_data.setIndexData(index_data_device_address)
        .setIndexType(VK_INDEX_TYPE_UINT32);
  }

  blas_.addGeometry(
      scene::AccelerationStructure::GeometryData()
          // AABB
          //.setAABBsData(scene::AccelerationStructure::AABBsData())
          // Instances
          //.setInstancesData(scene::AccelerationStructure::InstancesData())
          // Triangles
          .setTrianglesData(triangles_data)
          // Flags
          .setFlags(VK_GEOMETRY_OPAQUE_BIT_KHR)
          // Geometry Type
          .setType(VK_GEOMETRY_TYPE_TRIANGLES_KHR),
      tracer_object.primitive_count, tracer_object.transform_offset);

  return *this;
}

VeResult RayTracer::createPipeline(VkDevice vk_device) {
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      descriptor_set_layout_,
      DescriptorSet::Layout::Config()
          .addLayoutBinding(0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1,
                            VK_SHADER_STAGE_RAYGEN_BIT_KHR)
          .addLayoutBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1,
                            VK_SHADER_STAGE_RAYGEN_BIT_KHR)
          .addLayoutBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
                            VK_SHADER_STAGE_RAYGEN_BIT_KHR)
          .build(vk_device));

  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      pipeline_layout_, Pipeline::Layout::Config()
                            .addDescriptorSetLayout(*descriptor_set_layout_)
                            .build(vk_device));

  std::filesystem::path shaders_path(VENUS_SHADERS_PATH);
  ShaderModule raygen, miss, chit;
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      raygen, ShaderModule::Config()
                  .fromSpvFile(shaders_path / "raygen.rgen.spv")
                  .build(vk_device));
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      miss, ShaderModule::Config()
                .fromSpvFile(shaders_path / "miss.rmiss.spv")
                .build(vk_device));
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      chit, ShaderModule::Config()
                .fromSpvFile(shaders_path / "closesthit.rchit.spv")
                .build(vk_device));

  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      pipeline_,
      RayTracingPipeline::Config()
          .addShaderStage(Pipeline::ShaderStage()
                              .setStages(VK_SHADER_STAGE_RAYGEN_BIT_KHR)
                              .build(raygen))
          .addShaderStage(Pipeline::ShaderStage()
                              .setStages(VK_SHADER_STAGE_MISS_BIT_KHR)
                              .build(miss))
          .addShaderStage(Pipeline::ShaderStage()
                              .setStages(VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR)
                              .build(chit))
          .addShaderGroup(
              RayTracingPipeline::ShaderGroup()
                  .setType(VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR)
                  .setGeneralShader(0))
          .addShaderGroup(
              RayTracingPipeline::ShaderGroup()
                  .setType(VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR)
                  .setGeneralShader(1))
          .addShaderGroup(
              RayTracingPipeline::ShaderGroup()
                  .setType(
                      VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR)
                  .setClosestHitShader(2))
          .build(vk_device, *pipeline_layout_));

  return VeResult::noError();
}

VeResult RayTracer::createShaderBindingTable(const core::Device &device) {
  ray_tracing_pipeline_properties_ = device.physical().rayTracingProperties();
  const uint32_t handle_size =
      ray_tracing_pipeline_properties_.shaderGroupHandleSize;
  const uint32_t handle_size_aligned = hermes::mem::alignment::alignedSize(
      ray_tracing_pipeline_properties_.shaderGroupHandleSize,
      ray_tracing_pipeline_properties_.shaderGroupHandleAlignment);
  // const uint32_t handle_alignment =
  //     ray_tracing_pipeline_properties.shaderGroupHandleAlignment;
  const uint32_t group_count =
      static_cast<uint32_t>(pipeline_.shaderGroups().size());
  const uint32_t sbt_size = group_count * handle_size_aligned;

  // Raygen
  // Create binding table buffers for each shader type
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      raygen_sbt_,
      mem::AllocatedBuffer::Config::forShaderBindingTable(handle_size)
          .build(device));
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      miss_sbt_,
      mem::AllocatedBuffer::Config::forShaderBindingTable(handle_size)
          .build(device));
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      hit_sbt_, mem::AllocatedBuffer::Config::forShaderBindingTable(handle_size)
                    .build(device));

  std::vector<uint8_t> shader_handle_storage(sbt_size);
  VENUS_VK_RETURN_BAD_RESULT(vkGetRayTracingShaderGroupHandlesKHR(
      *device, *pipeline_, 0, group_count, sbt_size,
      shader_handle_storage.data()));
  {
    VENUS_DECLARE_OR_RETURN_BAD_RESULT(mem::DeviceMemory::ScopedMap, m,
                                       raygen_sbt_.scopedMap());
    auto *data = m.get<u8 *>();
    memcpy(data, shader_handle_storage.data(), handle_size);
  }
  {
    VENUS_DECLARE_OR_RETURN_BAD_RESULT(mem::DeviceMemory::ScopedMap, m,
                                       miss_sbt_.scopedMap());
    auto *data = m.get<u8 *>();
    memcpy(data, shader_handle_storage.data() + handle_size_aligned,
           handle_size);
  }
  {
    VENUS_DECLARE_OR_RETURN_BAD_RESULT(mem::DeviceMemory::ScopedMap, m,
                                       hit_sbt_.scopedMap());
    auto *data = m.get<u8 *>();
    memcpy(data, shader_handle_storage.data() + handle_size_aligned * 2,
           handle_size);
  }

  return VeResult::noError();
}

VeResult RayTracer::createDescriptorSets(VkDevice vk_device) {
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      descriptor_allocator_,
      pipeline::DescriptorAllocator::Config()
          .setInitialSetCount(1)
          .addDescriptorType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1.f)
          .addDescriptorType(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1.f)
          .addDescriptorType(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.f)
          .build(vk_device));

  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      descriptor_set_, descriptor_allocator_.allocate(*descriptor_set_layout_));
  // Setup the descriptor for binding our top level acceleration structure to
  // the ray tracing shaders

  DescriptorWriter()
      .writeAccelerationStructure(0, *tlas_)
      .writeImage(1, *image_view_, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL,
                  VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
      .writeBuffer(2, *ubo_, sizeof(RayTracer::UniformBuffer), 0,
                   VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
      .update(descriptor_set_);

  return VeResult::noError();
}

VeResult RayTracer::prepare(const engine::GraphicsDevice &gd,
                            VkQueue vk_queue) {
  // setup output image
  if (!image_ || resolution_.width != image_.resolution().width ||
      resolution_.height != image_.resolution().height) {
    VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
        image_,
        mem::AllocatedImage::Config::forStorage(resolution_).build(*gd));
    VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
        image_view_,
        mem::Image::View::Config()
            .setViewType(VK_IMAGE_VIEW_TYPE_2D)
            .setFormat(image_.format())
            .setSubresourceRange({VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1})
            .build(image_));
    // transition image to GENERAL
    VENUS_RETURN_BAD_RESULT(
        gd.immediateSubmit([&](const pipeline::CommandBuffer &cb) {
          cb.transitionImage(*image_, VK_IMAGE_LAYOUT_UNDEFINED,
                             VK_IMAGE_LAYOUT_GENERAL);
        }));
  }

  // BLAS

  blas_.setType(VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR);
  VENUS_RETURN_BAD_RESULT(
      blas_.build(gd,                                                        //
                  vk_queue,                                                  //
                  VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR, //
                  VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR));

  // TLAS

  VkTransformMatrixKHR transform_matrix = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                                           0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};

  VkAccelerationStructureInstanceKHR acceleration_structure_instance{};
  acceleration_structure_instance.transform = transform_matrix;
  acceleration_structure_instance.instanceCustomIndex = 0;
  acceleration_structure_instance.mask = 0xFF;
  acceleration_structure_instance.instanceShaderBindingTableRecordOffset = 0;
  acceleration_structure_instance.flags =
      VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
  acceleration_structure_instance.accelerationStructureReference =
      blas_.deviceAddress();

  mem::AllocatedBuffer instances_buffer;
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      instances_buffer,
      mem::AllocatedBuffer::Config::forAccelerationStructure(
          sizeof(VkAccelerationStructureInstanceKHR))
          .addUsage(
              VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR)
          .build(*gd));
  VENUS_RETURN_BAD_RESULT(
      instances_buffer.copy(&acceleration_structure_instance,
                            sizeof(VkAccelerationStructureInstanceKHR)));

  VkDeviceOrHostAddressConstKHR instance_data_device_address{};
  instance_data_device_address.deviceAddress = instances_buffer.deviceAddress();

  tlas_.addGeometry(
      scene::AccelerationStructure::GeometryData()
          .setType(VK_GEOMETRY_TYPE_INSTANCES_KHR)
          .setFlags(VK_GEOMETRY_OPAQUE_BIT_KHR)
          // instances data
          .setInstancesData(scene::AccelerationStructure::InstancesData()
                                .setArrayOfPointers(false)
                                .setData(instance_data_device_address)),
      1, 0);

  tlas_.setType(VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR);
  VENUS_RETURN_BAD_RESULT(
      tlas_.build(gd,                                                        //
                  vk_queue,                                                  //
                  VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR, //
                  VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR));

  if (!ubo_) {
    VENUS_ASSIGN_OR_RETURN_BAD_RESULT(ubo_,
                                      mem::AllocatedBuffer::Config::forUniform(
                                          sizeof(RayTracer::UniformBuffer))
                                          .build(*gd));
  }

  VENUS_RETURN_BAD_RESULT(createPipeline(**gd));
  VENUS_RETURN_BAD_RESULT(createShaderBindingTable(*gd));
  VENUS_RETURN_BAD_RESULT(createDescriptorSets(**gd));

  // update ubo
  scene::Camera camera =
      scene::Camera::perspective(60.f).setPosition({0.f, 0.f, -2.5f});
  camera.projection()->setNear(0.1f).setFar(512.f);
  RayTracer::UniformBuffer ubo_data;
  ubo_data.proj_inverse = hermes::math::transpose(
      hermes::math::inverse(camera.projectionTransform().matrix()));
  ubo_data.view_inverse = hermes::math::transpose(
      hermes::math::inverse(camera.viewTransform().matrix()));
  {
    VENUS_DECLARE_OR_RETURN_BAD_RESULT(mem::DeviceMemory::ScopedMap, m,
                                       ubo_.scopedMap());
    memcpy(m.get<void *>(), &ubo_data, sizeof(RayTracer::UniformBuffer));
  }
  return VeResult::noError();
}

VeResult RayTracer::record(const CommandBuffer &cb,
                           VkImage vk_color_image) const {
  /*
      Setup the strided device address regions pointing at the shader
     identifiers in the shader binding table
  */

  const uint32_t handle_size_aligned = hermes::mem::alignment::alignedSize(
      ray_tracing_pipeline_properties_.shaderGroupHandleSize,
      ray_tracing_pipeline_properties_.shaderGroupHandleAlignment);

  VkStridedDeviceAddressRegionKHR raygen_shader_sbt_entry{};
  raygen_shader_sbt_entry.deviceAddress = raygen_sbt_.deviceAddress();
  raygen_shader_sbt_entry.stride = handle_size_aligned;
  raygen_shader_sbt_entry.size = handle_size_aligned;

  VkStridedDeviceAddressRegionKHR miss_shader_sbt_entry{};
  miss_shader_sbt_entry.deviceAddress = miss_sbt_.deviceAddress();
  miss_shader_sbt_entry.stride = handle_size_aligned;
  miss_shader_sbt_entry.size = handle_size_aligned;

  VkStridedDeviceAddressRegionKHR hit_shader_sbt_entry{};
  hit_shader_sbt_entry.deviceAddress = hit_sbt_.deviceAddress();
  hit_shader_sbt_entry.stride = handle_size_aligned;
  hit_shader_sbt_entry.size = handle_size_aligned;

  VkStridedDeviceAddressRegionKHR callable_shader_sbt_entry{};

  cb.bindPipeline(*pipeline_, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
  cb.bind(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, *pipeline_layout_, 0,
          {*descriptor_set_}, {});

  cb.traceRays(&raygen_shader_sbt_entry, &miss_shader_sbt_entry,
               &hit_shader_sbt_entry, &callable_shader_sbt_entry,
               image_.resolution().width, image_.resolution().height, 1);

  VkImageSubresourceRange subresource_range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1,
                                               0, 1};

  // Prepare current swap chain image as transfer destination
  cb.transitionImageLayout(
      vk_color_image, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      VK_PIPELINE_STAGE_TRANSFER_BIT, {}, VK_ACCESS_TRANSFER_WRITE_BIT,
      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      subresource_range);

  // Prepare ray tracing output image as transfer source
  cb.transitionImageLayout(*image_, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                           VK_PIPELINE_STAGE_TRANSFER_BIT, {},
                           VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           subresource_range);

  VkImageCopy copy_region{};
  copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
  copy_region.srcOffset = {0, 0, 0};
  copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
  copy_region.dstOffset = {0, 0, 0};
  copy_region.extent = image_.resolution();

  cb.copy(*image_, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vk_color_image,
          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, {copy_region});

  cb.transitionImage(vk_color_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                     VK_IMAGE_LAYOUT_GENERAL);
  cb.transitionImage(vk_color_image, VK_IMAGE_LAYOUT_GENERAL,
                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

  cb.transitionImage(*image_, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                     VK_IMAGE_LAYOUT_GENERAL);

  return VeResult::noError();
}

} // namespace venus::pipeline
