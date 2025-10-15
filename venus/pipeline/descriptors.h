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

      Result<DescriptorSet::Layout> create(VkDevice vk_device,
                                           void *next = nullptr) const;

    private:
      std::vector<VkDescriptorSetLayoutBinding> bindings_;

      VENUS_to_string_FRIEND(Config);
    };

    // raii

    VENUS_DECLARE_RAII_FUNCTIONS(Layout)

    void destroy() noexcept;
    void swap(Layout &rhs) noexcept;

    VkDescriptorSetLayout operator*() const;

  private:
    VkDescriptorSetLayout vk_layout_{VK_NULL_HANDLE};
    VkDevice vk_device_{VK_NULL_HANDLE};

#ifdef VENUS_DEBUG
    Config config_;
#endif

    VENUS_to_string_FRIEND(Layout);
  };

  VENUS_DECLARE_RAII_FUNCTIONS(DescriptorSet)

  void destroy() noexcept;
  void swap(DescriptorSet &rhs) noexcept;

  VkDescriptorSet operator*() const;
  VkDevice device() const;

private:
  friend class DescriptorAllocator;

  VkDescriptorSet vk_descriptor_set_{VK_NULL_HANDLE};
  VkDescriptorPool vk_descriptor_pool_{VK_NULL_HANDLE};
  VkDevice vk_device_{VK_NULL_HANDLE};

  VENUS_to_string_FRIEND(DescriptorSet);
};

/// Manages the allocation of device memory for storing descriptor sets. The
/// descriptor allocator generates descriptor pools as more allocation
/// requests come in. Each pool is created based on the defined the list pool
/// size ratios, that estimates the distribution of descriptor types in each
/// pool.
/// \note Pools are created on demand.
/// \note This uses raii.
class DescriptorAllocator {
  /// Describes the distribution of a descriptor type inside a pool.
  struct PoolSizeRatio {
    VkDescriptorType type; //< descriptor type
    f32 ratio;             //< distribution estimate
  };

public:
  /// Builder for descriptor allocator.
  struct Config {
    /// Estimate of sets per created pool.
    /// \param set_count
    Config &setInitialSetCount(u32 set_count);
    /// \param type
    /// \param ratio [def=1] Distribution of this descriptor type in each pool.
    Config &addDescriptorType(VkDescriptorType type, f32 ratio = 1.f);

    /// \brief Initializes with a single descriptor pool.
    Result<DescriptorAllocator> create(VkDevice device) const;

  private:
    u32 initial_set_count_{0};
    std::vector<PoolSizeRatio> ratios_;

    VENUS_to_string_FRIEND(DescriptorAllocator::Config);
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
  allocate(VkDescriptorSetLayout vk_layout);

private:
  // create a new pool
  Result<VkDescriptorPool> create(u32 set_count,
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

  VENUS_to_string_FRIEND(DescriptorAllocator);
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
  DescriptorWriter &writeBuffer(i32 binding, VkBuffer buffer, u32 size,
                                u32 offset, VkDescriptorType type);
  /// \brief Registers VkWriteDescriptorSet for a given image.
  /// \param binding
  /// \param image
  /// \param sampler
  /// \param layout
  /// \param type Descriptor type.
  DescriptorWriter &writeImage(i32 binding, VkImageView image,
                               VkSampler sampler, VkImageLayout layout,
                               VkDescriptorType type);
  /// Clears all registered writes.
  void clear();
  /// \brief Updates the descriptor set with all registered writes.
  /// \param device
  /// \param set The descriptor set to update.
  DescriptorWriter &update(const DescriptorSet &set);

private:
  std::deque<VkDescriptorImageInfo> image_infos_;
  std::deque<VkDescriptorBufferInfo> buffer_infos_;
  std::vector<VkWriteDescriptorSet> writes_;
};

} // namespace venus::pipeline
