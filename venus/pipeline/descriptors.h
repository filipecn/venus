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

/// \file   descriptors.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30
/// \brief  Classes for manipulating descriptor sets.

#pragma once

#include <venus/core/device.h>

#include <deque>
#include <span>

namespace venus::pipeline {

/// A descriptor set is a set of resources that are bound into the pipeline
/// as a group. Multiple sets can be bound to a pipeline at a time. Each set
/// has layout, which describes the order and types of resources in the set.
/// \note Descriptor sets can only be constructed by descriptor allocators.
/// \note This class uses RAII.
class DescriptorSet {
public:
  /// \note This class uses RAII.
  class Layout {
  public:
    struct Config {
      /// Resources are bound to binding points in the descriptor set. Each
      /// binding is described by the parameters of this method.
      /// \param binding Each resource accessible to a shader is given a binding
      ///        number.
      /// \param descriptor_type
      /// \param descriptor_count
      /// \param stage_flags
      Config &addLayoutBinding(u32 binding, VkDescriptorType type,
                               u32 descritor_count,
                               VkShaderStageFlags stage_flags);

      Result<DescriptorSet::Layout> build(VkDevice vk_device,
                                          void *next = nullptr) const;

    private:
      std::vector<VkDescriptorSetLayoutBinding> bindings_;

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
      friend struct hermes::DebugTraits<DescriptorSet::Layout::Config>;
#endif
    };

    // raii

    VENUS_DECLARE_RAII_FUNCTIONS(Layout)

    void destroy() noexcept;
    void swap(Layout &rhs) noexcept;

    operator bool() const;
    VkDescriptorSetLayout operator*() const;

  private:
    VkDescriptorSetLayout vk_layout_{VK_NULL_HANDLE};
    VkDevice vk_device_{VK_NULL_HANDLE};

#ifdef VENUS_DEBUG
    Config config_;
#endif

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
    friend struct hermes::DebugTraits<DescriptorSet::Layout>;
#endif
  };

  VENUS_DECLARE_RAII_FUNCTIONS(DescriptorSet)

  void destroy() noexcept;
  void swap(DescriptorSet &rhs) noexcept;

  operator bool() const;
  VkDescriptorSet operator*() const;
  VkDevice device() const;

private:
  friend class DescriptorAllocator;

  VkDescriptorSet vk_descriptor_set_{VK_NULL_HANDLE};
  VkDescriptorPool vk_descriptor_pool_{VK_NULL_HANDLE};
  VkDevice vk_device_{VK_NULL_HANDLE};

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
  friend struct hermes::DebugTraits<DescriptorSet>;
#endif
};

/// Manages the allocation of device memory for storing descriptor sets. The
/// descriptor allocator generates descriptor pools as more allocation
/// requests come in. Each pool is created based on the defined the list pool
/// size ratios, that estimates the distribution of descriptor types in each
/// pool.
/// \note Pools are created on demand.
/// \note This uses raii.
class DescriptorAllocator {
public:
  /// Describes the distribution of a descriptor type inside a pool.
  struct PoolSizeRatio {
    VkDescriptorType type; //< descriptor type
    f32 ratio;             //< distribution estimate
  };

  /// Builder for descriptor allocator.
  struct Config {
    /// Estimate of sets per created pool.
    /// \param set_count
    Config &setInitialSetCount(u32 set_count);
    /// \param type
    /// \param ratio [def=1] Distribution of this descriptor type in each pool.
    Config &addDescriptorType(VkDescriptorType type, f32 ratio = 1.f);

    /// \brief Initializes with a single descriptor pool.
    Result<DescriptorAllocator> build(VkDevice device) const;

  private:
    u32 initial_set_count_{0};
    std::vector<PoolSizeRatio> ratios_;

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
    friend struct hermes::DebugTraits<DescriptorAllocator::Config>;
#endif
  };

  // raii

  VENUS_DECLARE_RAII_FUNCTIONS(DescriptorAllocator)

  /// \brief Resets all underlying pools.
  void reset(VkDescriptorPoolResetFlags reset_flags = {});
  /// \brief Destroys all underlying pools.
  void destroy() noexcept;
  void swap(DescriptorAllocator &rhs) noexcept;
  /// Allocates memory for a given descriptor set with a given layout.
  /// \param device
  /// \param layout descriptor set layout.
  HERMES_NODISCARD Result<DescriptorSet>
  allocate(VkDescriptorSetLayout vk_layout, void *next = nullptr);

private:
  // create a new pool
  Result<VkDescriptorPool> build(u32 set_count,
                                 std::span<PoolSizeRatio> pool_ratios);
  // get the current ready pool (or create a new one if necessary)
  Result<VkDescriptorPool> get();

  VkDevice vk_device_{VK_NULL_HANDLE};
  /// Estimate distribution of descriptor types per pool.
  std::vector<PoolSizeRatio> ratios_;
  /// Pools that can't allocate anymore.
  std::vector<VkDescriptorPool> full_pools_;
  /// Empty pools ready to be used.
  std::vector<VkDescriptorPool> ready_pools_;
  /// Pools capacity
  u32 sets_per_pool_{0};

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
  friend struct hermes::DebugTraits<DescriptorAllocator>;
#endif
};

/// The descriptor writer updates data of allocated descriptors in device
/// memory so shaders can access during execution. This works by registering
/// VkWriteDescriptorSet objects and writing them to descriptor sets later.
/// \note This class is utilized by materials to generate material instances.
class DescriptorWriter {
public:
  /// \brief Registers VkWriteDescriptorSet for a given buffer.
  /// \param binding
  /// \param buffer
  /// \param size
  /// \param offset
  /// \param type Descriptor type.
  DescriptorWriter &writeBuffer(i32 binding, VkBuffer vk_buffer, u32 size,
                                u32 offset, VkDescriptorType type);
  /// \brief Registers VkWriteDescriptorSet for a given image.
  /// \param binding
  /// \param vk_image_view
  /// \param vk_sampler
  /// \param vk_image_layout
  /// \param type Descriptor type.
  DescriptorWriter &writeImage(i32 binding, VkImageView vk_image_view,
                               VkSampler vk_sampler,
                               VkImageLayout vk_image_layout,
                               VkDescriptorType type);
  /// \brief Registers VkWriteDescriptorSet for a given set of images.
  /// \param binding
  /// \param images
  /// \param type Descriptor type.
  DescriptorWriter &
  writeImages(i32 binding, const std::vector<VkDescriptorImageInfo> &images,
              VkDescriptorType type);
  /// Registers VkWriteDescriptorSetAccelerationStructure for a given AS.
  /// \param binding
  /// \param vk_acceleration_structure
  DescriptorWriter &writeAccelerationStructure(
      i32 binding, VkAccelerationStructureKHR vk_acceleration_structure);
  /// Clears all registered writes.
  void clear();
  /// \brief Updates the descriptor set with all registered writes.
  /// \param set The descriptor set to update.
  DescriptorWriter &update(const DescriptorSet &set);
  /// \brief Updates the descriptor set with all registered writes.
  /// \param vk_device
  /// \param vk_set The descriptor set to update.
  DescriptorWriter &update(VkDevice vk_device, VkDescriptorSet vk_set);

private:
  std::deque<VkDescriptorImageInfo> image_infos_;
  std::deque<VkDescriptorBufferInfo> buffer_infos_;
  // TODO: support multiple acceleration structure writes
  VkAccelerationStructureKHR acceleration_structure_{VK_NULL_HANDLE};
  VkWriteDescriptorSetAccelerationStructureKHR acceleration_structure_info_{};
  std::vector<VkWriteDescriptorSet> writes_;
};

} // namespace venus::pipeline

#ifdef VENUS_INCLUDE_DEBUG_TRAITS

namespace hermes {

template <> struct DebugTraits<venus::pipeline::DescriptorSet::Layout::Config> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage
  message(const venus::pipeline::DescriptorSet::Layout::Config &data) {
    return DebugMessage()
        .addTitle("DescriptorSet Layout Config")
        .addArray("bindings", data.bindings_);
  }
};

template <> struct DebugTraits<venus::pipeline::DescriptorSet::Layout> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage
  message(const venus::pipeline::DescriptorSet::Layout &data) {
    return DebugMessage()
        .addTitle("DescriptorSet Layout")
        .add("vk_layout", VENUS_VK_HANDLE_STRING(data.vk_layout_))
        .add("vk_device", VENUS_VK_DISPATCHABLE_HANDLE_STRING(data.vk_device_))
        .add("config", data.config_);
  }
};

template <> struct DebugTraits<venus::pipeline::DescriptorSet> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage message(const venus::pipeline::DescriptorSet &data) {
    return DebugMessage()
        .addTitle("DescriptorSet")
        .add("vk_descriptor_set",
             VENUS_VK_HANDLE_STRING(data.vk_descriptor_set_))
        .add("vk_descriptor_pool",
             VENUS_VK_HANDLE_STRING(data.vk_descriptor_pool_))
        .add("vk_device", VENUS_VK_DISPATCHABLE_HANDLE_STRING(data.vk_device_));
  }
};

template <>
struct DebugTraits<venus::pipeline::DescriptorAllocator::PoolSizeRatio> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage
  message(const venus::pipeline::DescriptorAllocator::PoolSizeRatio &data) {
    return DebugMessage("({} {})", VENUS_VK_STRING(VkDescriptorType, data.type),
                        data.ratio);
  }
};

template <> struct DebugTraits<venus::pipeline::DescriptorAllocator::Config> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage
  message(const venus::pipeline::DescriptorAllocator::Config &data) {
    return DebugMessage()
        .addTitle("Descriptor Allocator Config")
        .add("initial_set_count", data.initial_set_count_)
        .addArray("ratios", data.ratios_);
  }
};

template <> struct DebugTraits<venus::pipeline::DescriptorAllocator> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage
  message(const venus::pipeline::DescriptorAllocator &data) {
    return DebugMessage()
        .addTitle("Descriptor Allocator")
        .add("vk_device", VENUS_VK_DISPATCHABLE_HANDLE_STRING(data.vk_device_))
        .add("sets_per_pool", data.sets_per_pool_)
        .addArray("ratios", data.ratios_)
        .add("full pools count", data.full_pools_.size())
        .add("ready pools count", data.ready_pools_.size());
  }
};

} // namespace hermes

#endif // VENUS_INCLUDE_DEBUG_TRAITS