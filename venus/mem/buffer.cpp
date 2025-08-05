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

Result<Buffer> Buffer::Config::create(const core::Device &device) const {
  return create(*device);
}

Result<Buffer> Buffer::Config::create(VkDevice vk_device) const {

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
  VENUS_VK_RETURN_BAD_RESULT(
      vkCreateBuffer(vk_device, &info, nullptr, &buffer.vk_buffer_));
  buffer.size_ = size_;
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
  core::vk::swap(vk_device_, rhs.vk_device_);
  core::vk::swap(vk_buffer_view_, rhs.vk_buffer_view_);
  return *this;
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

Buffer::~Buffer() { destroy(); }

Buffer &Buffer::operator=(Buffer &&rhs) noexcept {
  destroy();
  core::vk::swap(vk_buffer_, rhs.vk_buffer_);
  core::vk::swap(vk_device_, rhs.vk_device_);
  std::swap(size_, rhs.size_);
#ifdef VENUS_DEBUG
  config_ = rhs.config_;
#endif
  return *this;
}

void Buffer::destroy() noexcept {
  if (vk_device_ && vk_buffer_) {
    vkDestroyBuffer(vk_device_, vk_buffer_, nullptr);
  }
  vk_device_ = VK_NULL_HANDLE;
  vk_buffer_ = VK_NULL_HANDLE;
  size_ = 0;
}

Result<VkMemoryRequirements> Buffer::memoryRequirements() const {
  VkMemoryRequirements memory_requirements;
  vkGetBufferMemoryRequirements(vk_device_, vk_buffer_, &memory_requirements);
  return Result<VkMemoryRequirements>(std::move(memory_requirements));
}

VkDeviceSize Buffer::sizeInBytes() const { return size_; }

VkDeviceAddress Buffer::deviceAddress() const {
  VkBufferDeviceAddressInfo info{};
  info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
  info.pNext = nullptr;
  info.buffer = vk_buffer_;

  return vkGetBufferDeviceAddress(vk_device_, &info);
}

} // namespace venus::mem

namespace venus {
HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::mem::Buffer::Config)
HERMES_PUSH_DEBUG_TITLE
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::mem::Buffer)
HERMES_PUSH_DEBUG_TITLE
HERMES_TO_STRING_DEBUG_METHOD_END

} // namespace venus
