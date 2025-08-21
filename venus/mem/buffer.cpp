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

Buffer::Config &Buffer::Config::setSize(VkDeviceSize size_in_bytes) {
  HERMES_CHECK(size_in_bytes);
  size_ = size_in_bytes;
  return *this;
}

Buffer::Config &Buffer::Config::addUsage(VkBufferUsageFlags usage) {
  usage_ |= usage;
  return *this;
}

Buffer::Config &Buffer::Config::setSharingMode(VkSharingMode mode) {
  sharing_mode_ = mode;
  return *this;
}

Buffer::Config &Buffer::Config::enableShaderDeviceAddress() {
  usage_ |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
  return *this;
}

Buffer::Config &Buffer::Config::addCreateFlags(VkBufferCreateFlags flags) {
  flags_ = flags;
  return *this;
}

Buffer::Config &
Buffer::Config::setAllocated(const VmaAllocationCreateInfo &memory_info) {
  allocation_ = memory_info;
  return *this;
}

Result<Buffer> Buffer::Config::create(const core::Device &device) const {
  VkBufferCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  info.pNext = nullptr;
  info.flags = flags_;
  info.size = size_;
  info.usage = usage_;
  info.sharingMode = sharing_mode_;
  info.queueFamilyIndexCount = 0;
  info.pQueueFamilyIndices = nullptr;

  Buffer buffer;

  if (allocation_.has_value()) {
    auto alloc_info = allocation_.value();
    VmaInfo vma_info;
    vma_info.allocator = device.allocator();
    VENUS_VK_RETURN_BAD_RESULT(vmaCreateBuffer(device.allocator(), &info,
                                               &alloc_info, &buffer.vk_buffer_,
                                               &vma_info.allocation, nullptr));
    buffer.info_ = vma_info;
  } else {
    VENUS_VK_RETURN_BAD_RESULT(
        vkCreateBuffer(*device, &info, nullptr, &buffer.vk_buffer_));

    VkInfo vk_info;
    vk_info.vk_device = *device;
    vk_info.size = size_;
    buffer.info_ = vk_info;

    VkBufferDeviceAddressInfo info{};
    info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    info.pNext = nullptr;
    info.buffer = buffer.vk_buffer_;
    buffer.vk_device_address_ = vkGetBufferDeviceAddress(*device, &info);
  }

#ifdef VENUS_DEBUG
  buffer.config_ = *this;
#endif

  return Result<Buffer>(std::move(buffer));
}

Buffer::View::Config &Buffer::View::Config::setFormat(VkFormat format) {
  format_ = format;
  return *this;
}

Buffer::View::Config &Buffer::View::Config::setRange(VkDeviceSize range) {
  range_ = range;
  return *this;
}

Buffer::View::Config &Buffer::View::Config::setOffset(VkDeviceSize offset) {
  offset_ = offset;
  return *this;
}

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

// helper type for the visitor
template <class... Ts> struct overloads : Ts... {
  using Ts::operator()...;
};

void Buffer::swap(Buffer &rhs) noexcept {
  VENUS_SWAP_FIELD_WITH_RHS(vk_buffer_);
  VENUS_SWAP_FIELD_WITH_RHS(info_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_device_address_);
#ifdef VENUS_DEBUG
  VENUS_SWAP_FIELD_WITH_RHS(config_);
#endif
}

void Buffer::destroy() noexcept {
  std::visit(overloads{
                 [&](VmaInfo &info) {
                   if (info.allocator && info.allocation)
                     vmaDestroyBuffer(info.allocator, vk_buffer_,
                                      info.allocation);
                   info.allocation = VK_NULL_HANDLE;
                 },
                 [&](VkInfo &info) {
                   if (info.vk_device && vk_buffer_) {
                     vkDestroyBuffer(info.vk_device, vk_buffer_, nullptr);
                     info.vk_device = VK_NULL_HANDLE;
                     info.size = 0;
                   }
                 },
             },
             info_);
  vk_buffer_ = VK_NULL_HANDLE;
  vk_device_address_ = 0;
}

Result<VkMemoryRequirements> Buffer::memoryRequirements() const {
  VkMemoryRequirements memory_requirements;
  std::visit(
      overloads{
          [&](const VmaInfo &info) { memory_requirements = info.requirements; },
          [&](const VkInfo &info) {
            vkGetBufferMemoryRequirements(info.vk_device, vk_buffer_,
                                          &memory_requirements);
          },
      },
      info_);
  return Result<VkMemoryRequirements>(std::move(memory_requirements));
}

VkDeviceAddress Buffer::deviceAddress() const { return vk_device_address_; }

} // namespace venus::mem

namespace venus {
HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::mem::Buffer::Config)
HERMES_PUSH_DEBUG_TITLE
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::mem::Buffer)
HERMES_PUSH_DEBUG_TITLE
HERMES_TO_STRING_DEBUG_METHOD_END

} // namespace venus
