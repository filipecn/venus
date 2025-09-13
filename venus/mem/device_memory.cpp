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

namespace venus::mem {

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(DeviceMemory, setAllocationFlags,
                                     VmaAllocationCreateFlags,
                                     vma_allocation_.flags |= value);
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(DeviceMemory, setUsage, VmaMemoryUsage,
                                     vma_allocation_.usage = value);
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(DeviceMemory, addRequiredProperties,
                                     VkMemoryPropertyFlags,
                                     vma_allocation_.requiredFlags |= value);
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(DeviceMemory, addPreferredProperties,
                                     VkMemoryPropertyFlags,
                                     vma_allocation_.preferredFlags |= value);
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(DeviceMemory, addMemoryType, u32,
                                     vma_allocation_.memoryTypeBits |= value);
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(DeviceMemory, setPool, VmaPool,
                                     vma_allocation_.pool = value);
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(DeviceMemory, setPriority, f32,
                                     vma_allocation_.priority = value);
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(DeviceMemory, setRequirements,
                                     const VkMemoryRequirements &,
                                     requirements_ = value);

DeviceMemory::Config &DeviceMemory::Config::setHostVisible() {
  vma_allocation_.requiredFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
  return *this;
}

DeviceMemory::Config &DeviceMemory::Config::setDeviceLocal() {
  vma_allocation_.requiredFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  return *this;
}

VmaAllocationCreateInfo DeviceMemory::Config::allocationInfo() const {
  return vma_allocation_;
}

Result<DeviceMemory> DeviceMemory::Config::create(const core::Device &device) {
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

  VENUS_VK_RETURN_BAD_RESULT(
      vmaAllocateMemory(device.allocator(), &requirements_, &vma_allocation_,
                        &device_memory.vma_allocation_, nullptr));

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
  swap(rhs);
  return *this;
}

Result<void *> DeviceMemory::map(VkDeviceSize size, VkDeviceSize offset,
                                 VkMemoryMapFlags flags) const {
  if (mapped_) {
    HERMES_ERROR("Mapping an already mapped device memory. A device memory can "
                 "only have one mapped memory at a time.");
    return VeResult::badAllocation();
  }
  VENUS_VK_RETURN_BAD_RESULT(
      vmaMapMemory(vma_allocator_, vma_allocation_, &mapped_));
  // vkMapMemory(vk_device_, vk_device_memory_, offset,
  //                                        size ? size : size_, flags,
  //                                        &mapped_));

  return Result<void *>(mapped_);
}

VeResult DeviceMemory::access(const std::function<void(void *)> &f,
                              VkDeviceSize size_in_bytes, VkDeviceSize offset,
                              VkMemoryMapFlags flags) const {
  void *m = nullptr;
  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      m, this->map(size_in_bytes, offset, flags));
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

  void *dst{nullptr};
  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(dst,
                                           map(size_in_bytes, offset, flags));

  std::memcpy(dst, data, size_in_bytes);
  unmap();
  return VeResult::noError();
}

VkDeviceSize DeviceMemory::size() const { return vma_allocation_->GetSize(); }

} // namespace venus::mem

namespace venus {
HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::mem::DeviceMemory::Config)
HERMES_PUSH_DEBUG_TITLE
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::mem::DeviceMemory)
HERMES_PUSH_DEBUG_TITLE
HERMES_TO_STRING_DEBUG_METHOD_END

} // namespace venus
