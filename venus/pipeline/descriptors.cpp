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

/// \file   descriptors.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30

#include <venus/pipeline/descriptors.h>

#include <venus/utils/vk_debug.h>

#include <numeric>

namespace venus::pipeline {

DescriptorSet::Layout::Config &DescriptorSet::Layout::Config::addLayoutBinding(
    u32 binding, VkDescriptorType type, u32 descritor_count,
    VkShaderStageFlags stage_flags) {
  VkDescriptorSetLayoutBinding info;
  info.binding = binding;
  info.stageFlags = stage_flags;
  info.descriptorCount = descritor_count;
  info.descriptorType = type;
  info.pImmutableSamplers = nullptr;
  bindings_.emplace_back(info);
  return *this;
}

Result<DescriptorSet::Layout>
DescriptorSet::Layout::Config::create(VkDevice vk_device, void *next) const {

  VkDescriptorSetLayoutCreateInfo info;
  info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  info.flags = {};
  info.pNext = next;
  info.bindingCount = bindings_.size();
  info.pBindings = bindings_.data();

  DescriptorSet::Layout layout;
  VENUS_VK_RETURN_BAD_RESULT(vkCreateDescriptorSetLayout(
      vk_device, &info, nullptr, &layout.vk_layout_));
  layout.vk_device_ = vk_device;
#ifdef VENUS_DEBUG
  layout.config_ = *this;
#endif

  return Result<DescriptorSet::Layout>(std::move(layout));
}

DescriptorSet::Layout::Layout(DescriptorSet::Layout &&rhs) noexcept {
  *this = std::move(rhs);
}

DescriptorSet::Layout::~Layout() noexcept { destroy(); }

DescriptorSet::Layout &
DescriptorSet::Layout::operator=(DescriptorSet::Layout &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void DescriptorSet::Layout::swap(DescriptorSet::Layout &rhs) noexcept {
  VENUS_SWAP_FIELD_WITH_RHS(vk_device_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_layout_);
#ifdef VENUS_DEBUG
  VENUS_SWAP_FIELD_WITH_RHS(config_);
#endif
}

void DescriptorSet::Layout::destroy() noexcept {
  if (vk_device_ && vk_layout_)
    vkDestroyDescriptorSetLayout(vk_device_, vk_layout_, nullptr);
  vk_device_ = VK_NULL_HANDLE;
  vk_layout_ = VK_NULL_HANDLE;
}

VkDescriptorSetLayout DescriptorSet::Layout::operator*() const {
  return vk_layout_;
}

DescriptorSet::DescriptorSet(DescriptorSet &&rhs) noexcept {
  *this = std::move(rhs);
}

DescriptorSet::~DescriptorSet() noexcept { destroy(); }

DescriptorSet &DescriptorSet::operator=(DescriptorSet &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void DescriptorSet::swap(DescriptorSet &rhs) noexcept {
  VENUS_SWAP_FIELD_WITH_RHS(vk_device_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_descriptor_pool_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_descriptor_set_);
}

void DescriptorSet::destroy() noexcept {}

VkDescriptorSet DescriptorSet::operator*() const { return vk_descriptor_set_; }

VkDevice DescriptorSet::device() const { return vk_device_; }

DescriptorAllocator::Config &
DescriptorAllocator::Config::setInitialSetCount(u32 set_count) {
  initial_set_count_ = set_count;
  return *this;
}

DescriptorAllocator::Config &
DescriptorAllocator::Config::addDescriptorType(VkDescriptorType type,
                                               f32 ratio) {
  DescriptorAllocator::PoolSizeRatio sr;
  sr.type = type;
  sr.ratio = ratio;
  ratios_.emplace_back(sr);
  return *this;
}

Result<DescriptorAllocator>
DescriptorAllocator::Config::create(VkDevice vk_device) const {
  DescriptorAllocator da;
  da.vk_device_ = vk_device;
  da.sets_per_pool_ = initial_set_count_ * 1.5;

  for (auto r : ratios_)
    da.ratios_.push_back(r);

  VkDescriptorPool new_pool{VK_NULL_HANDLE};

  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      new_pool, da.create(initial_set_count_, da.ratios_));

  da.ready_pools_.emplace_back(std::move(new_pool));

  return Result<DescriptorAllocator>(std::move(da));
}

DescriptorAllocator::DescriptorAllocator(DescriptorAllocator &&rhs) noexcept {
  *this = std::move(rhs);
}

DescriptorAllocator::~DescriptorAllocator() noexcept { destroy(); }

DescriptorAllocator &
DescriptorAllocator::operator=(DescriptorAllocator &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void DescriptorAllocator::swap(DescriptorAllocator &rhs) noexcept {
  VENUS_SWAP_FIELD_WITH_RHS(vk_device_);
  VENUS_SWAP_FIELD_WITH_RHS(ratios_);
  VENUS_SWAP_FIELD_WITH_RHS(full_pools_);
  VENUS_SWAP_FIELD_WITH_RHS(ready_pools_);
  VENUS_SWAP_FIELD_WITH_RHS(sets_per_pool_);
}

void DescriptorAllocator::destroy() noexcept {
  if (vk_device_) {
    for (auto &pool : full_pools_)
      vkDestroyDescriptorPool(vk_device_, pool, nullptr);
    for (auto &pool : ready_pools_)
      vkDestroyDescriptorPool(vk_device_, pool, nullptr);
  }
  full_pools_.clear();
  ready_pools_.clear();
  ratios_.clear();
}

void DescriptorAllocator::reset(VkDescriptorPoolResetFlags flags) {
  if (vk_device_) {
    for (auto &pool : ready_pools_)
      vkResetDescriptorPool(vk_device_, pool, flags);
    for (auto &pool : full_pools_) {
      vkResetDescriptorPool(vk_device_, pool, flags);
      ready_pools_.emplace_back(std::move(pool));
    }
    full_pools_.clear();
  }
}

Result<VkDescriptorPool> DescriptorAllocator::create(
    u32 set_count, std::span<DescriptorAllocator::PoolSizeRatio> pool_ratios) {
  std::vector<VkDescriptorPoolSize> pool_sizes;
  for (auto r : pool_ratios)
    pool_sizes.push_back({r.type, static_cast<u32>(r.ratio * set_count)});

  u32 max_sets = std::accumulate(pool_sizes.begin(), pool_sizes.end(), 0,
                                 [](u32 sum, const VkDescriptorPoolSize &dps) {
                                   return sum + dps.descriptorCount;
                                 });

  if (max_sets == 0)
    return VeResult::inputError();

  VkDescriptorPoolCreateInfo info;
  info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  info.pNext = nullptr;
  info.maxSets = max_sets;
  info.pPoolSizes = pool_sizes.data();
  info.poolSizeCount = pool_sizes.size();

  VkDescriptorPool vk_pool{VK_NULL_HANDLE};
  VENUS_VK_RETURN_BAD_RESULT(
      vkCreateDescriptorPool(vk_device_, &info, nullptr, &vk_pool));
  return Result<VkDescriptorPool>(std::move(vk_pool));
}

Result<VkDescriptorPool> DescriptorAllocator::get() {
  VkDescriptorPool new_pool{VK_NULL_HANDLE};
  if (ready_pools_.empty()) {
    VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(new_pool,
                                             create(sets_per_pool_, ratios_));
    sets_per_pool_ = std::min(static_cast<u32>(sets_per_pool_ * 1.5), 4092u);
  } else {
    new_pool = std::move(ready_pools_.back());
    ready_pools_.pop_back();
  }
  return Result<VkDescriptorPool>(std::move(new_pool));
}

Result<DescriptorSet>
DescriptorAllocator::allocate(VkDescriptorSetLayout vk_layout) {
  DescriptorSet descriptor_set;
  descriptor_set.vk_device_ = vk_device_;

  VkDescriptorPool pool{VK_NULL_HANDLE};
  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(pool, get());
  VkDescriptorSetAllocateInfo info;
  info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  info.pNext = nullptr;
  info.descriptorSetCount = 1;
  info.pSetLayouts = &vk_layout;
  info.descriptorPool = pool;
  auto err = vkAllocateDescriptorSets(vk_device_, &info,
                                      &descriptor_set.vk_descriptor_set_);

  switch (err) {
  case VK_SUCCESS:
    descriptor_set.vk_descriptor_pool_ = pool;
    ready_pools_.emplace_back(pool);
    return Result<DescriptorSet>(std::move(descriptor_set));
  case VK_ERROR_FRAGMENTED_POOL:
  case VK_ERROR_OUT_OF_DEVICE_MEMORY:
  case VK_ERROR_OUT_OF_HOST_MEMORY:
  case VK_ERROR_OUT_OF_POOL_MEMORY:
  case VK_ERROR_UNKNOWN:
  case VK_ERROR_VALIDATION_FAILED_EXT:
    break;
  default:
    full_pools_.emplace_back(std::move(pool));
    VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(pool, get());
    VENUS_VK_RETURN_BAD_RESULT(vkAllocateDescriptorSets(
        vk_device_, &info, &descriptor_set.vk_descriptor_set_));
    ready_pools_.emplace_back(std::move(pool));
    return Result<DescriptorSet>(std::move(descriptor_set));
  }
  return VeResult::badAllocation();
}

DescriptorWriter &DescriptorWriter::writeBuffer(i32 binding, VkBuffer buffer,
                                                u32 size, u32 offset,
                                                VkDescriptorType type) {
  buffer_infos_.emplace_back(buffer, offset, size);
  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.pNext = nullptr;
  write.pBufferInfo = &buffer_infos_.back();
  write.descriptorType = type;
  write.descriptorCount = 1;
  write.dstBinding = binding;
  writes_.push_back(write);
  return *this;
}

DescriptorWriter &DescriptorWriter::writeImage(i32 binding, VkImageView image,
                                               VkSampler sampler,
                                               VkImageLayout layout,
                                               VkDescriptorType type) {
  VkDescriptorImageInfo info;
  info.imageView = image;
  info.sampler = sampler;
  info.imageLayout = layout;
  image_infos_.emplace_back(info);
  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.pNext = nullptr;
  write.pImageInfo = &image_infos_.back();
  write.descriptorType = type;
  write.descriptorCount = 1;
  write.dstBinding = binding;
  writes_.push_back(write);
  return *this;
}

void DescriptorWriter::clear() {
  image_infos_.clear();
  buffer_infos_.clear();
  writes_.clear();
}

DescriptorWriter &DescriptorWriter::update(const DescriptorSet &set) {
  for (auto &write : writes_)
    write.dstSet = *set;

  vkUpdateDescriptorSets(set.device(), writes_.size(), writes_.data(), 0,
                         nullptr);
  return *this;
}

} // namespace venus::pipeline

namespace venus {
HERMES_TO_STRING_DEBUG_METHOD_BEGIN(
    venus::pipeline::DescriptorSet::Layout::Config)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_ARRAY_FIELD_BEGIN(bindings_, binding)
HERMES_PUSH_DEBUG_LINE("{} {} {} {}", binding.binding,
                       string_VkDescriptorType(binding.descriptorType),
                       binding.descriptorCount,
                       string_VkShaderStageFlags(binding.stageFlags))
HERMES_PUSH_DEBUG_ARRAY_FIELD_END
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::pipeline::DescriptorSet::Layout)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_VK_FIELD(vk_layout_)
HERMES_PUSH_DEBUG_VK_FIELD(vk_device_)
HERMES_PUSH_DEBUG_VENUS_FIELD(config_)
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::pipeline::DescriptorSet)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_VK_FIELD(vk_descriptor_set_)
HERMES_PUSH_DEBUG_VK_FIELD(vk_descriptor_pool_)
HERMES_PUSH_DEBUG_VK_FIELD(vk_device_)
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(
    venus::pipeline::DescriptorAllocator::Config)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_FIELD(initial_set_count_)
HERMES_PUSH_DEBUG_ARRAY_FIELD_BEGIN(ratios_, ratio)
HERMES_UNUSED_VARIABLE(ratio)
HERMES_PUSH_DEBUG_VK_STRING(VkDescriptorType, ratios_[i].type)
HERMES_PUSH_DEBUG_FIELD(ratios_[i].ratio)
HERMES_PUSH_DEBUG_ARRAY_FIELD_END
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::pipeline::DescriptorAllocator)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_VK_FIELD(vk_device_)
HERMES_PUSH_DEBUG_FIELD(sets_per_pool_)
HERMES_PUSH_DEBUG_ARRAY_FIELD_BEGIN(ratios_, ratio)
HERMES_UNUSED_VARIABLE(ratio)
HERMES_PUSH_DEBUG_VK_STRING(VkDescriptorType, ratios_[i].type)
HERMES_PUSH_DEBUG_FIELD(ratios_[i].ratio)
HERMES_PUSH_DEBUG_ARRAY_FIELD_END
HERMES_PUSH_DEBUG_ARRAY_FIELD_BEGIN(full_pools_, pool)
HERMES_UNUSED_VARIABLE(pool)
HERMES_PUSH_DEBUG_VK_FIELD(full_pools_[i])
HERMES_PUSH_DEBUG_ARRAY_FIELD_END
HERMES_PUSH_DEBUG_ARRAY_FIELD_BEGIN(ready_pools_, pool)
HERMES_UNUSED_VARIABLE(pool)
HERMES_PUSH_DEBUG_VK_FIELD(ready_pools_[i])
HERMES_PUSH_DEBUG_ARRAY_FIELD_END
HERMES_TO_STRING_DEBUG_METHOD_END

} // namespace venus
