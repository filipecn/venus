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

/// \file   acceleration_structure.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30
/// \brief  Vulkan Acceleration Structure

#pragma once

#include <venus/engine/graphics_device.h>
#include <venus/mem/buffer.h>

namespace venus::scene {

/// Acceleration structures are an implementation-dependent opaque
/// representation of geometric objects, which are used for ray tracing. By
/// building objects into acceleration structures, ray tracing can be performed
/// against a known data layout, and in an efficient manner.
/// \note This class uses RAII.
class AccelerationStructure {
public:
  /// \note Wrapper for VkAccelerationStructureGeometryInstancesDataKHR
  struct InstancesData {
    InstancesData() noexcept;
    InstancesData &setArrayOfPointers(VkBool32 is_array_of_pointers = true);
    InstancesData &setData(VkDeviceOrHostAddressConstKHR data);
    VkAccelerationStructureGeometryInstancesDataKHR operator*() const;

  private:
    VkAccelerationStructureGeometryInstancesDataKHR info_{};
  };
  /// \note Wrapper for VkAccelerationStructureGeometryAabbsDataKHR
  struct AABBsData {
    AABBsData() noexcept;
    AABBsData &setData(VkDeviceOrHostAddressConstKHR data);
    AABBsData &setStride(VkDeviceSize stride);
    VkAccelerationStructureGeometryAabbsDataKHR operator*() const;

  private:
    VkAccelerationStructureGeometryAabbsDataKHR info_{};
  };
  /// Structure specifying a triangle geometry in a bottom-level acceleration
  /// structure
  /// Default values are:
  /// vertex_format = VK_FORMAT_R32G32B32_SFLOAT
  /// index_type    = VK_INDEX_TYPE_NONE
  /// \note Wrapper for VkAccelerationStructureGeometryTrianglesDataKHR
  /// \note If index buffer if given, then index_type must be defined.
  struct TrianglesData {
    TrianglesData() noexcept;
    TrianglesData &setVertexFormat(VkFormat vertex_format);
    TrianglesData &setVertexData(VkDeviceOrHostAddressConstKHR vertex_data);
    TrianglesData &setVertexStride(VkDeviceSize vertex_stride);
    TrianglesData &setMaxVertex(uint32_t max_vertex);
    TrianglesData &setIndexType(VkIndexType index_type);
    TrianglesData &setIndexData(VkDeviceOrHostAddressConstKHR index_data);
    TrianglesData &
    setTransformData(VkDeviceOrHostAddressConstKHR transform_data);
    VkAccelerationStructureGeometryTrianglesDataKHR operator*() const;

  private:
    VkAccelerationStructureGeometryTrianglesDataKHR info_{};
  };
  /// Default values are:
  /// flags = VK_GEOMETRY_OPAQUE_BIT_KHR
  /// wrapper for VkAccelerationStructureGeometryKHR
  struct GeometryData {
    GeometryData() noexcept;
    GeometryData &setType(VkGeometryTypeKHR type);
    GeometryData &setTrianglesData(const TrianglesData &triangles_data);
    GeometryData &setInstancesData(const InstancesData &instances_data);
    GeometryData &setAABBsData(const AABBsData &aabbs_data);
    GeometryData &setFlags(VkGeometryFlagsKHR flags);
    VkAccelerationStructureGeometryKHR operator*() const;

  private:
    VkAccelerationStructureGeometryKHR info_{};
  };

  VENUS_DECLARE_RAII_FUNCTIONS(AccelerationStructure)
  void destroy() noexcept;
  void swap(AccelerationStructure &rhs);

  VkAccelerationStructureKHR operator*() const;
  VkDevice device() const;

  AccelerationStructure &addGeometry(const GeometryData &geometry_data,
                                     u32 primitive_count, u32 transform_offset);

  AccelerationStructure &updateGeometry(u64 index,
                                        const GeometryData &geometry_data,
                                        u32 primitive_count,
                                        u32 transform_offset);
  AccelerationStructure &setType(VkAccelerationStructureTypeKHR type);

  VeResult build(const engine::GraphicsDevice &gd, VkQueue vk_queue,
                 VkBuildAccelerationStructureFlagsKHR flags,
                 VkBuildAccelerationStructureModeKHR mode);

  VkDeviceAddress deviceAddress() const;

private:
  struct Geometry {
    VkAccelerationStructureGeometryKHR geometry{};
    u32 primitive_count{};
    u32 transform_offset{};
    bool updated = false;
  };

  VkAccelerationStructureKHR vk_acceleration_structure_{VK_NULL_HANDLE};
  VkDevice vk_device_{VK_NULL_HANDLE};

  u64 device_address_{0};
  VkAccelerationStructureTypeKHR type_{};
  VkAccelerationStructureBuildSizesInfoKHR build_sizes_info_{};

  mem::AllocatedBuffer buffer_;
  std::unordered_map<u32, Geometry> geometries_;

  VENUS_to_string_FRIEND(AccelerationStructure);
};

} // namespace venus::scene
