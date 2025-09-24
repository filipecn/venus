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

/// \file   buffer.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30
/// \brief  Vulkan buffer

#include <venus/mem/buffer.h>

#include <venus/utils/vk_debug.h>

namespace venus::mem {

Buffer::Config Buffer::Config::forStaging(const VkDeviceSize &size_in_bytes) {
  return Buffer::Config()
      .addUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
      .setSize(size_in_bytes);
}

Buffer::Config Buffer::Config::forUniform(const VkDeviceSize &size_in_bytes) {
  return Buffer::Config()
      .addUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
      .addUsage(VK_BUFFER_USAGE_TRANSFER_DST_BIT)
      .setSize(size_in_bytes);
}

Buffer::Config Buffer::Config::forStorage(const VkDeviceSize &size_in_bytes) {
  return Buffer::Config()
      .addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
      .addUsage(VK_BUFFER_USAGE_TRANSFER_DST_BIT)
      .setSize(size_in_bytes);
}

Buffer::Config Buffer::Config::forIndices(u32 index_count,
                                          VkIndexType index_type) {
  return Buffer::Config()
      .addUsage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
      .addUsage(VK_BUFFER_USAGE_TRANSFER_DST_BIT)
      .setSize(index_count * core::vk::indexSize(index_type));
}

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(Buffer, setSize, VkDeviceSize,
                                     size_ = value)

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(Buffer, addUsage, VkBufferUsageFlags,
                                     usage_ |= value)

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(Buffer, setSharingMode, VkSharingMode,
                                     sharing_mode_ = value)

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(Buffer, addCreateFlags,
                                     VkBufferCreateFlags, flags_ = value)

Buffer::Config &Buffer::Config::enableShaderDeviceAddress() {
  usage_ |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
  return *this;
}

VkBufferCreateInfo Buffer::Config::createInfo() const {
  VkBufferCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  info.pNext = nullptr;
  info.flags = flags_;
  info.size = size_;
  info.usage = usage_;
  info.sharingMode = sharing_mode_;
  info.queueFamilyIndexCount = 0;
  info.pQueueFamilyIndices = nullptr;
  return info;
}

Result<Buffer> Buffer::Config::create(const core::Device &device) const {
  auto info = createInfo();

  VkBuffer vk_buffer{VK_NULL_HANDLE};
  VENUS_VK_RETURN_BAD_RESULT(
      vkCreateBuffer(*device, &info, nullptr, &vk_buffer));

  Buffer buffer;

  if (usage_ & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
    buffer.vk_device_address_ = 0;

  buffer.init(*device, vk_buffer);

#ifdef VENUS_DEBUG
  buffer.config_ = *this;
#endif

  return Result<Buffer>(std::move(buffer));
}

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(Buffer::View, setFormat, VkFormat,
                                     format_ = value)

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(Buffer::View, setRange, VkDeviceSize,
                                     range_ = value)

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(Buffer::View, setOffset, VkDeviceSize,
                                     offset_ = value)

Result<Buffer::View> Buffer::View::Config::create(const Buffer &buffer) const {
  VkBufferViewCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
  info.pNext = nullptr;
  info.flags = 0;
  info.buffer = *buffer;
  info.format = format_;
  info.offset = offset_;
  info.range = range_;

  Buffer::View view;
  VENUS_VK_RETURN_BAD_RESULT(vkCreateBufferView(buffer.device(), &info, nullptr,
                                                &view.vk_buffer_view_));
  return Result<Buffer::View>(std::move(view));
}

Buffer::View::View(Buffer::View &&rhs) noexcept { *this = std::move(rhs); }

Buffer::View::~View() noexcept { destroy(); }

Buffer::View &Buffer::View::operator=(Buffer::View &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void Buffer::View::swap(Buffer::View &rhs) noexcept {
  VENUS_SWAP_FIELD_WITH_RHS(vk_device_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_buffer_view_);
}

void Buffer::View::destroy() noexcept {
  if (vk_device_ && vk_buffer_view_) {
    vkDestroyBufferView(vk_device_, vk_buffer_view_, nullptr);
  }
  vk_device_ = VK_NULL_HANDLE;
  vk_buffer_view_ = VK_NULL_HANDLE;
}

VkBufferView Buffer::View::operator*() const { return vk_buffer_view_; }

Buffer::Buffer(Buffer &&rhs) noexcept { *this = std::move(rhs); }

Buffer::~Buffer() noexcept { destroy(); }

Buffer &Buffer::operator=(Buffer &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void Buffer::swap(Buffer &rhs) noexcept {
  VENUS_SWAP_FIELD_WITH_RHS(vk_buffer_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_device_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_device_address_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_memory_requirements_);
#ifdef VENUS_DEBUG
  VENUS_SWAP_FIELD_WITH_RHS(config_);
#endif
}

void Buffer::destroy() noexcept {
  if (vk_device_ && vk_buffer_) {
    vkDestroyBuffer(vk_device_, vk_buffer_, nullptr);
  }
  vk_buffer_ = VK_NULL_HANDLE;
  vk_device_ = VK_NULL_HANDLE;
  vk_device_address_ = 0;
  vk_memory_requirements_ = {};
}

VkMemoryRequirements Buffer::memoryRequirements() const {
  return vk_memory_requirements_;
}

VkDeviceSize Buffer::sizeInBytes() const {
  return vk_memory_requirements_.size;
}

VkDeviceAddress Buffer::deviceAddress() const {
  if (vk_device_address_.has_value())
    return vk_device_address_.value();
  HERMES_ERROR("Trying to access buffer address. Buffer Address not enabled.");
  return 0;
}

VkBuffer Buffer::operator*() const { return vk_buffer_; }

VkDevice Buffer::device() const { return vk_device_; }

void Buffer::init(VkDevice vk_device, VkBuffer vk_buffer) {
  vk_device_ = vk_device;
  vk_buffer_ = vk_buffer;
  vkGetBufferMemoryRequirements(vk_device_, vk_buffer_,
                                &vk_memory_requirements_);
  if (vk_device_address_.has_value()) {
    VkBufferDeviceAddressInfo da_info{};
    da_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    da_info.pNext = nullptr;
    da_info.buffer = vk_buffer_;
    vk_device_address_ = vkGetBufferDeviceAddress(vk_device_, &da_info);
  }
}

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(AllocatedBuffer, setBufferConfig,
                                     const Buffer::Config &,
                                     buffer_config_ = value)

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(AllocatedBuffer, setMemoryConfig,
                                     const DeviceMemory::Config &,
                                     mem_config_ = value)

Result<AllocatedBuffer>
AllocatedBuffer::Config::create(const core::Device &device) const {
  auto info = buffer_config_.createInfo();

  auto alloc_info = mem_config_.allocationInfo();

  VkBuffer vk_buffer{VK_NULL_HANDLE};
  VmaAllocation vma_allocation{VK_NULL_HANDLE};
  VENUS_VK_RETURN_BAD_RESULT(vmaCreateBuffer(device.allocator(), &info,
                                             &alloc_info, &vk_buffer,
                                             &vma_allocation, nullptr));

  AllocatedBuffer buffer;
  buffer.vma_allocator_ = device.allocator();
  buffer.vma_allocation_ = vma_allocation;

  if (info.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
    buffer.vk_device_address_ = 0;

  buffer.init(*device, vk_buffer);

  return Result<AllocatedBuffer>(std::move(buffer));
}

AllocatedBuffer::AllocatedBuffer(AllocatedBuffer &&rhs) noexcept {
  *this = std::move(rhs);
}

AllocatedBuffer::~AllocatedBuffer() noexcept { destroy(); }

AllocatedBuffer &AllocatedBuffer::operator=(AllocatedBuffer &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void AllocatedBuffer::swap(AllocatedBuffer &rhs) noexcept {
  VENUS_SWAP_FIELD_WITH_RHS(vk_buffer_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_device_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_device_address_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_memory_requirements_);
  VENUS_SWAP_FIELD_WITH_RHS(vma_allocator_);
  VENUS_SWAP_FIELD_WITH_RHS(vma_allocation_);
}

void AllocatedBuffer::destroy() noexcept {
  if (vma_allocator_ && vma_allocation_)
    vmaDestroyBuffer(vma_allocator_, vk_buffer_, vma_allocation_);
  vma_allocation_ = VK_NULL_HANDLE;
  vk_device_ = VK_NULL_HANDLE;
  vk_buffer_ = VK_NULL_HANDLE;
  vk_memory_requirements_ = {};
}

} // namespace venus::mem

namespace venus {

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::mem::Buffer::Config)
HERMES_PUSH_DEBUG_TITLE
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::mem::Buffer)
HERMES_PUSH_DEBUG_TITLE
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::mem::AllocatedBuffer::Config)
HERMES_PUSH_DEBUG_TITLE
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::mem::AllocatedBuffer)
HERMES_PUSH_DEBUG_TITLE
HERMES_TO_STRING_DEBUG_METHOD_END

} // namespace venus
