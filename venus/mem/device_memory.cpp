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

/// \file   device_memory.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30

#include <venus/mem/device_memory.h>

#include <venus/utils/vk_debug.h>

namespace venus::mem {

DeviceMemory::Config &
DeviceMemory::Config::addProperties(VkMemoryPropertyFlags properties) {
  properties_ |= properties;
  return *this;
}

DeviceMemory::Config &
DeviceMemory::Config::setProperties(VkMemoryPropertyFlags properties) {
  properties_ = properties;
  return *this;
}

DeviceMemory::Config &DeviceMemory::Config::setRequirements(
    const VkMemoryRequirements &requirements) {
  requirements_ = requirements;
  return *this;
}

DeviceMemory::Config &DeviceMemory::Config::setHostVisible() {
  properties_ = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
  return *this;
}

DeviceMemory::Config &DeviceMemory::Config::setDeviceLocal() {
  properties_ = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  return *this;
}

DeviceMemory::Config &DeviceMemory::Config::addAllocationFlags(
    VkMemoryAllocateFlags allocation_flags) {
  allocation_flags_ |= allocation_flags;
  return *this;
}

Result<DeviceMemory> DeviceMemory::Config::create(const core::Device &device) {
  if (!requirements_.size)
    return VeResult::badAllocation();

  VkMemoryAllocateFlagsInfo ext{};
  ext.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
  ext.flags = allocation_flags_;

  VkMemoryAllocateInfo allocate_info{};
  allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocate_info.pNext = &ext;
  allocate_info.allocationSize = requirements_.size;
  allocate_info.memoryTypeIndex =
      device.physical().chooseMemoryType(requirements_, properties_, {});

  DeviceMemory device_memory;
  VENUS_VK_RETURN_BAD_RESULT(vkAllocateMemory(
      *device, &allocate_info, nullptr, &device_memory.vk_device_memory_));
  device_memory.size_ = requirements_.size;
  device_memory.vk_device_ = *device;
#ifdef VENUS_DEBUG
  device_memory.config_ = *this;
#endif
  return Result<DeviceMemory>(std::move(device_memory));
}

DeviceMemory::DeviceMemory(DeviceMemory &&rhs) noexcept {
  *this = std::move(rhs);
}

DeviceMemory::~DeviceMemory() noexcept { destroy(); }

DeviceMemory &DeviceMemory::operator=(DeviceMemory &&rhs) noexcept {
  destroy();
  core::vk::swap(vk_device_memory_, rhs.vk_device_memory_);
  core::vk::swap(vk_device_, rhs.vk_device_);
  std::swap(size_, rhs.size_);
#ifdef VENUS_DEBUG
  config_ = std::move(rhs.config_);
#endif
  return *this;
}

Result<void *> DeviceMemory::map(VkDeviceSize size, VkDeviceSize offset,
                                 VkMemoryMapFlags flags) const {
  if (mapped_) {
    HERMES_ERROR("Mapping an already mapped device memory. A device memory can "
                 "only have one mapped memory at a time.");
    return VeResult::badAllocation();
  }

  VENUS_VK_RETURN_BAD_RESULT(vkMapMemory(vk_device_, vk_device_memory_, offset,
                                         size ? size : size_, flags, &mapped_));

  return Result<void *>(mapped_);
}

void DeviceMemory::unmap() const {
  if (mapped_) {
    vkUnmapMemory(vk_device_, vk_device_memory_);
    mapped_ = nullptr;
  }
}

VeResult DeviceMemory::flush(VkDeviceSize size, VkDeviceSize offset) {
  VkMappedMemoryRange mapped_range = {};
  mapped_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  mapped_range.memory = vk_device_memory_;
  mapped_range.offset = offset;
  mapped_range.size = size;

  VENUS_VK_RETURN_BAD_RESULT(
      vkFlushMappedMemoryRanges(vk_device_, 1, &mapped_range));
  return VeResult::noError();
}

VeResult DeviceMemory::invalidate(VkDeviceSize size, VkDeviceSize offset) {
  VkMappedMemoryRange mapped_range = {};
  mapped_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  mapped_range.memory = vk_device_memory_;
  mapped_range.offset = offset;
  mapped_range.size = size;
  VENUS_VK_RETURN_BAD_RESULT(
      vkInvalidateMappedMemoryRanges(vk_device_, 1, &mapped_range));
  return VeResult::noError();
}

void DeviceMemory::destroy() noexcept {
  unmap();
  if (vk_device_ && vk_device_memory_)
    vkFreeMemory(vk_device_, vk_device_memory_, nullptr);
  vk_device_memory_ = VK_NULL_HANDLE;
  vk_device_ = VK_NULL_HANDLE;
  size_ = 0;
}

VeResult DeviceMemory::copy(const void *data, VkDeviceSize size_in_bytes,
                            VkDeviceSize offset, VkMemoryMapFlags flags) {
  if (offset + size_in_bytes > size_) {
    return VeResult::inputError();
  }

  void *dst{nullptr};
  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(dst,
                                           map(size_in_bytes, offset, flags));

  std::memcpy(dst, data, size_in_bytes);
  unmap();
  return VeResult::noError();
}

VkDeviceMemory DeviceMemory::operator*() const { return vk_device_memory_; }

VkDevice DeviceMemory::device() const { return vk_device_; }

VkDeviceSize DeviceMemory::size() const { return size_; }

} // namespace venus::mem

namespace venus {
HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::mem::DeviceMemory::Config)
HERMES_PUSH_DEBUG_TITLE
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::mem::DeviceMemory)
HERMES_PUSH_DEBUG_TITLE
HERMES_TO_STRING_DEBUG_METHOD_END

} // namespace venus
