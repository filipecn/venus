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

/// \file   acceleration_structure.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-11-11

#include <venus/scene/acceleration_structure.h>

#include <venus/utils/vk_debug.h>

namespace venus::scene {

AccelerationStructure::InstancesData::InstancesData() noexcept {
  info_.sType =
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
  info_.pNext = nullptr;
  info_.arrayOfPointers = false;
}

VENUS_DEFINE_SET_INFO_FIELD_METHOD(AccelerationStructure::InstancesData,
                                   setArrayOfPointers, VkBool32,
                                   arrayOfPointers)
VENUS_DEFINE_SET_INFO_FIELD_METHOD(AccelerationStructure::InstancesData,
                                   setData, VkDeviceOrHostAddressConstKHR, data)

VkAccelerationStructureGeometryInstancesDataKHR
AccelerationStructure::InstancesData::operator*() const {
  return info_;
}

AccelerationStructure::AABBsData::AABBsData() noexcept {
  info_.sType =
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
  info_.pNext = nullptr;
}

VENUS_DEFINE_SET_INFO_FIELD_METHOD(AccelerationStructure::AABBsData, setData,
                                   VkDeviceOrHostAddressConstKHR, data)
VENUS_DEFINE_SET_INFO_FIELD_METHOD(AccelerationStructure::AABBsData, setStride,
                                   VkDeviceSize, stride)

VkAccelerationStructureGeometryAabbsDataKHR
AccelerationStructure::AABBsData::operator*() const {
  return info_;
}

AccelerationStructure::TrianglesData::TrianglesData() noexcept {
  info_.sType =
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
  info_.pNext = nullptr;
  info_.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
  info_.indexType = VK_INDEX_TYPE_UINT32;
}

VENUS_DEFINE_SET_INFO_FIELD_METHOD(AccelerationStructure::TrianglesData,
                                   setVertexFormat, VkFormat, vertexFormat)

VENUS_DEFINE_SET_INFO_FIELD_METHOD(AccelerationStructure::TrianglesData,
                                   setVertexData, VkDeviceOrHostAddressConstKHR,
                                   vertexData);
VENUS_DEFINE_SET_INFO_FIELD_METHOD(AccelerationStructure::TrianglesData,
                                   setVertexStride, VkDeviceSize, vertexStride);
VENUS_DEFINE_SET_INFO_FIELD_METHOD(AccelerationStructure::TrianglesData,
                                   setMaxVertex, uint32_t, maxVertex);
VENUS_DEFINE_SET_INFO_FIELD_METHOD(AccelerationStructure::TrianglesData,
                                   setIndexType, VkIndexType, indexType);
VENUS_DEFINE_SET_INFO_FIELD_METHOD(AccelerationStructure::TrianglesData,
                                   setIndexData, VkDeviceOrHostAddressConstKHR,
                                   indexData);
VENUS_DEFINE_SET_INFO_FIELD_METHOD(AccelerationStructure::TrianglesData,
                                   setTransformData,
                                   VkDeviceOrHostAddressConstKHR,
                                   transformData);

VkAccelerationStructureGeometryTrianglesDataKHR
AccelerationStructure::TrianglesData::operator*() const {
  return info_;
}

AccelerationStructure::GeometryData::GeometryData() noexcept {
  info_.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
  info_.pNext = nullptr;
  info_.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
}

VENUS_DEFINE_SET_INFO_FIELD_METHOD(AccelerationStructure::GeometryData, setType,
                                   VkGeometryTypeKHR, geometryType)
VENUS_DEFINE_SET_INFO_FIELD_METHOD(AccelerationStructure::GeometryData,
                                   setFlags, VkGeometryFlagsKHR, flags)

AccelerationStructure::GeometryData &
AccelerationStructure::GeometryData::setTrianglesData(
    const AccelerationStructure::TrianglesData &triangles_data) {
  info_.geometry.triangles = *triangles_data;
  return *this;
}

AccelerationStructure::GeometryData &
AccelerationStructure::GeometryData::setInstancesData(
    const AccelerationStructure::InstancesData &instances_data) {
  info_.geometry.instances = *instances_data;
  return *this;
}

AccelerationStructure::GeometryData &
AccelerationStructure::GeometryData::setAABBsData(
    const AccelerationStructure::AABBsData &aabbs_data) {
  info_.geometry.aabbs = *aabbs_data;
  return *this;
}

VkAccelerationStructureGeometryKHR
AccelerationStructure::GeometryData::operator*() const {
  return info_;
}

AccelerationStructure::AccelerationStructure(
    AccelerationStructure &&rhs) noexcept {
  *this = std::move(rhs);
}

AccelerationStructure::~AccelerationStructure() noexcept { destroy(); }

AccelerationStructure &
AccelerationStructure::operator=(AccelerationStructure &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void AccelerationStructure::destroy() noexcept {
  if (vk_acceleration_structure_ && vk_device_)
    vkDestroyAccelerationStructureKHR(vk_device_, vk_acceleration_structure_,
                                      nullptr);
  vk_device_ = VK_NULL_HANDLE;
  vk_acceleration_structure_ = VK_NULL_HANDLE;
  buffer_.destroy();
}

void AccelerationStructure::swap(AccelerationStructure &rhs) {
  VENUS_SWAP_FIELD_WITH_RHS(vk_device_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_acceleration_structure_);
  VENUS_SWAP_FIELD_WITH_RHS(buffer_);
}

VkAccelerationStructureKHR AccelerationStructure::operator*() const {
  return vk_acceleration_structure_;
}

VkDevice AccelerationStructure::device() const { return vk_device_; }

AccelerationStructure &AccelerationStructure::addGeometry(
    const AccelerationStructure::GeometryData &geometry_data,
    u32 primitive_count, u32 transform_offset) {
  geometries_[geometries_.size()] = {*geometry_data, primitive_count,
                                     transform_offset, false};
  return *this;
}

AccelerationStructure &AccelerationStructure::updateGeometry(
    u64 index, const AccelerationStructure::GeometryData &geometry_data,
    u32 primitive_count, u32 transform_offset) {
  geometries_[index] = {*geometry_data, primitive_count, transform_offset,
                        true};
  return *this;
}

VeResult
AccelerationStructure::build(const engine::GraphicsDevice &gd, VkQueue vk_queue,
                             VkBuildAccelerationStructureFlagsKHR flags,
                             VkBuildAccelerationStructureModeKHR mode) {
  HERMES_UNUSED_VARIABLE(vk_queue);

  const auto &device = *gd;

  std::vector<VkAccelerationStructureGeometryKHR>
      acceleration_structure_geometries;
  std::vector<VkAccelerationStructureBuildRangeInfoKHR>
      acceleration_structure_build_range_infos;
  std::vector<u32> primitive_counts;

  for (auto &geometry : geometries_) {
    if (mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR &&
        !geometry.second.updated) {
      continue;
    }
    acceleration_structure_geometries.push_back(geometry.second.geometry);
    // Infer build range info from geometry
    VkAccelerationStructureBuildRangeInfoKHR build_range_info;
    build_range_info.primitiveCount = geometry.second.primitive_count;
    build_range_info.primitiveOffset = 0;
    build_range_info.firstVertex = 0;
    build_range_info.transformOffset = geometry.second.transform_offset;
    acceleration_structure_build_range_infos.push_back(build_range_info);
    primitive_counts.push_back(geometry.second.primitive_count);
    geometry.second.updated = false;
  }

  VkAccelerationStructureBuildGeometryInfoKHR build_geometry_info{};
  build_geometry_info.sType =
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
  build_geometry_info.type = type_;
  build_geometry_info.flags = flags;
  build_geometry_info.mode = mode;
  if (mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR &&
      vk_acceleration_structure_ != VK_NULL_HANDLE) {
    build_geometry_info.srcAccelerationStructure = vk_acceleration_structure_;
    build_geometry_info.dstAccelerationStructure = vk_acceleration_structure_;
  }
  build_geometry_info.geometryCount =
      static_cast<uint32_t>(acceleration_structure_geometries.size());
  build_geometry_info.pGeometries = acceleration_structure_geometries.data();

  // Get required build sizes
  build_sizes_info_.sType =
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
  vkGetAccelerationStructureBuildSizesKHR(
      *device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
      &build_geometry_info, primitive_counts.data(), &build_sizes_info_);

  // Create a buffer for the acceleration structure
  if (*buffer_ == VK_NULL_HANDLE ||
      buffer_.sizeInBytes() != build_sizes_info_.accelerationStructureSize) {

    VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
        buffer_, mem::AllocatedBuffer::Config::forAccelerationStructure(
                     build_sizes_info_.accelerationStructureSize)
                     .build(device));

    VkAccelerationStructureCreateInfoKHR acceleration_structure_create_info{};
    acceleration_structure_create_info.sType =
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    acceleration_structure_create_info.buffer = *buffer_;
    acceleration_structure_create_info.size =
        build_sizes_info_.accelerationStructureSize;
    acceleration_structure_create_info.type = type_;

    VENUS_VK_RETURN_BAD_RESULT(vkCreateAccelerationStructureKHR(
        *device, &acceleration_structure_create_info, nullptr,
        &vk_acceleration_structure_));
  }

  // Get the acceleration structure's handle
  VkAccelerationStructureDeviceAddressInfoKHR
      acceleration_device_address_info{};
  acceleration_device_address_info.sType =
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
  acceleration_device_address_info.accelerationStructure =
      vk_acceleration_structure_;
  device_address_ = vkGetAccelerationStructureDeviceAddressKHR(
      *device, &acceleration_device_address_info);

  // Create a scratch buffer as a temporary storage for the acceleration
  // structure build
  mem::AllocatedBuffer scratch_buffer;
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(scratch_buffer,
                                    mem::AllocatedBuffer::Config::forStorage(
                                        build_sizes_info_.buildScratchSize, 0)
                                        .build(device));

  build_geometry_info.scratchData.deviceAddress =
      scratch_buffer.deviceAddress();
  build_geometry_info.dstAccelerationStructure = vk_acceleration_structure_;

  // Build the acceleration structure on the device via a one-time command
  // buffer submission

  auto as_build_range_infos = &*acceleration_structure_build_range_infos.data();

  VENUS_RETURN_BAD_RESULT(
      gd.immediateSubmit([&](const pipeline::CommandBuffer &cb) {
        vkCmdBuildAccelerationStructuresKHR(*cb, 1, &build_geometry_info,
                                            &as_build_range_infos);
      }));

  return VeResult::noError();
}

} // namespace venus::scene
