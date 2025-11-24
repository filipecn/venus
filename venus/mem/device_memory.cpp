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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#pragma GCC diagnostic pop

namespace venus {

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::mem::DeviceMemory::Config)
HERMES_PUSH_DEBUG_TITLE
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::mem::DeviceMemory)
HERMES_PUSH_DEBUG_TITLE
HERMES_TO_STRING_DEBUG_METHOD_END

} // namespace venus
  //
namespace venus::mem {

template <>
Result<DeviceMemory>
DeviceMemory::Setup<DeviceMemory::Config, DeviceMemory>::build(
    const core::Device &device) const {
  if (!requirements_.size)
    return VeResult::badAllocation();

  // Pure Vulkan
  // -----------
  // VkMemoryAllocateFlagsInfo ext{};
  // ext.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
  // ext.flags = vma_allocation_flags_;
  // VkMemoryAllocateInfo allocate_info{};
  // allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  // allocate_info.pNext = &ext;
  // allocate_info.allocationSize = requirements_.size;
  // allocate_info.memoryTypeIndex =
  //     device.physical().chooseMemoryType(requirements_, properties_, {});
  // VENUS_VK_RETURN_BAD_RESULT(vkAllocateMemory(
  //     *device, &allocate_info, nullptr, &device_memory.vk_device_memory_));

  DeviceMemory device_memory;

  VENUS_VK_RETURN_BAD_RESULT(vmaAllocateMemory(
      device.allocator(), &requirements_, &vma_allocation_create_info_,
      &device_memory.vma_allocation_, nullptr));
  device_memory.vma_allocator_ = device.allocator();

#ifdef VENUS_DEBUG
  device_memory.config_ = static_cast<const DeviceMemory::Config &>(*this);
#endif
  return Result<DeviceMemory>(std::move(device_memory));
}

DeviceMemory::ScopedMap::ScopedMap(DeviceMemory &memory, void *mapped) noexcept
    : memory_(memory), mapped_(mapped) {}

DeviceMemory::ScopedMap::~ScopedMap() noexcept { memory_.unmap(); }

DeviceMemory::DeviceMemory(DeviceMemory &&rhs) noexcept {
  *this = std::move(rhs);
}

DeviceMemory::~DeviceMemory() noexcept { destroy(); }

DeviceMemory &DeviceMemory::operator=(DeviceMemory &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

Result<DeviceMemory::ScopedMap> DeviceMemory::scopedMap() {
  void *mapped;
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(mapped, map());
  return Result<DeviceMemory::ScopedMap>(
      DeviceMemory::ScopedMap(*this, mapped));
}

Result<void *> DeviceMemory::map() const {
  if (mapped_) {
    HERMES_WARN("Mapping an already mapped device memory. A device memory can "
                "only have one mapped memory at a time.");
    return Result<void *>(mapped_);
  }
  VENUS_VK_RETURN_BAD_RESULT(
      vmaMapMemory(vma_allocator_, vma_allocation_, &mapped_));
  // vkMapMemory(vk_device_, vk_device_memory_, offset,
  //                                        size ? size : size_, flags,
  //                                        &mapped_));

  return Result<void *>(mapped_);
}

VeResult DeviceMemory::access(const std::function<void(void *)> &f) const {
  void *m = nullptr;
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(m, this->map());
  f(m);
  this->unmap();
  return VeResult::noError();
}

void DeviceMemory::unmap() const {
  if (mapped_) {
    vmaUnmapMemory(vma_allocator_, vma_allocation_);
    mapped_ = nullptr;
  }
}

VeResult DeviceMemory::flush(VkDeviceSize size, VkDeviceSize offset) {
  VENUS_VK_RETURN_BAD_RESULT(
      vmaFlushAllocation(vma_allocator_, vma_allocation_, offset, size));
  return VeResult::noError();
}

VeResult DeviceMemory::invalidate(VkDeviceSize size, VkDeviceSize offset) {
  VENUS_VK_RETURN_BAD_RESULT(
      vmaInvalidateAllocation(vma_allocator_, vma_allocation_, offset, size));
  return VeResult::noError();
}

void DeviceMemory::swap(DeviceMemory &rhs) noexcept {
  VENUS_SWAP_FIELD_WITH_RHS(vma_allocation_);
  VENUS_SWAP_FIELD_WITH_RHS(vma_allocator_);
#ifdef VENUS_DEBUG
  VENUS_SWAP_FIELD_WITH_RHS(config_);
#endif
}

void DeviceMemory::destroy() noexcept {
  unmap();
  if (vma_allocation_ && vma_allocator_)
    vmaFreeMemory(vma_allocator_, vma_allocation_);
  vma_allocator_ = VK_NULL_HANDLE;
  vma_allocation_ = VK_NULL_HANDLE;
}

VeResult DeviceMemory::copy(const void *data, VkDeviceSize size_in_bytes,
                            VkDeviceSize offset, VkMemoryMapFlags flags) {
  if (offset + size_in_bytes > vma_allocation_->GetSize()) {
    return VeResult::inputError();
  }

  HERMES_UNUSED_VARIABLE(flags);

  // void *dst{nullptr};
  // VENUS_ASSIGN_OR_RETURN_BAD_RESULT(dst,
  //                                         map(size_in_bytes, offset, flags));

  // std::memcpy(dst, data, size_in_bytes);

  VENUS_VK_RETURN_BAD_RESULT(vmaCopyMemoryToAllocation(
      vma_allocator_, data, vma_allocation_, offset, size_in_bytes));

  // unmap();
  return VeResult::noError();
}

VkDeviceSize DeviceMemory::size() const { return vma_allocation_->GetSize(); }

} // namespace venus::mem
