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

/// \file   sync.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30

#include <venus/core/sync.h>

#include <venus/utils/vk_debug.h>

namespace venus::core {

Fence::Config &Fence::Config::setCreateFlags(VkFenceCreateFlagBits flags) {
  flags_ |= flags;
  return *this;
}

Result<Fence> Fence::Config::create(VkDevice vk_device) const {
  VkFenceCreateInfo info;
  info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  info.pNext = nullptr;
  info.flags = flags_;

  Fence fence;
  fence.vk_device_ = vk_device;
  VENUS_VK_RETURN_BAD_RESULT(
      vkCreateFence(vk_device, &info, nullptr, &fence.vk_fence_));

  return Result<Fence>(std::move(fence));
}

Fence::Fence(Fence &&rhs) noexcept { *this = std::move(rhs); }

Fence &Fence::operator=(Fence &&rhs) noexcept {
  destroy();
  vk::swap(vk_fence_, rhs.vk_fence_);
  vk::swap(vk_device_, rhs.vk_device_);
  return *this;
}

Fence::~Fence() noexcept { destroy(); }

void Fence::destroy() noexcept {
  if (vk_device_ && vk_fence_)
    vkDestroyFence(vk_device_, vk_fence_, nullptr);
  vk_device_ = VK_NULL_HANDLE;
  vk_fence_ = VK_NULL_HANDLE;
}

VkFence Fence::operator*() const { return vk_fence_; }

VkResult Fence::status() const {
  return vkGetFenceStatus(vk_device_, vk_fence_);
}

void Fence::wait() const {
  vkWaitForFences(vk_device_, 1, &vk_fence_, VK_TRUE, UINT64_MAX);
}

void Fence::reset() const { vkResetFences(vk_device_, 1, &vk_fence_); }

Semaphore::Config &
Semaphore::Config::setCreateFlags(VkSemaphoreCreateFlags flags) {
  flags_ |= flags;
  return *this;
}

Result<Semaphore> Semaphore::Config::create(VkDevice vk_device) const {
  VkSemaphoreCreateInfo info;
  info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  info.pNext = nullptr;
  info.flags = flags_;

  Semaphore semaphore;
  semaphore.vk_device_ = vk_device;
  VENUS_VK_RETURN_BAD_RESULT(
      vkCreateSemaphore(vk_device, &info, nullptr, &semaphore.vk_semaphore_));

  return Result<Semaphore>(std::move(semaphore));
}

Semaphore::Semaphore(Semaphore &&rhs) noexcept { *this = std::move(rhs); }

Semaphore &Semaphore::operator=(Semaphore &&rhs) noexcept {
  destroy();
  vk::swap(vk_semaphore_, rhs.vk_semaphore_);
  vk::swap(vk_device_, rhs.vk_device_);
  return *this;
}

Semaphore::~Semaphore() noexcept { destroy(); }

void Semaphore::destroy() noexcept {
  if (vk_device_ && vk_semaphore_)
    vkDestroySemaphore(vk_device_, vk_semaphore_, nullptr);
  vk_device_ = VK_NULL_HANDLE;
  vk_semaphore_ = VK_NULL_HANDLE;
}

VkSemaphore Semaphore::operator*() const { return vk_semaphore_; }

} // namespace venus::core
