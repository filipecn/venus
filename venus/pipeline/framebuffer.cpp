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

/// \file   framebuffer.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30

#include <venus/pipeline/framebuffer.h>

#include <venus/utils/vk_debug.h>

namespace venus::pipeline {

Framebuffer::Config &
Framebuffer::Config::addAttachment(VkImageView image_view) {
  attachments_.emplace_back(image_view);
  return *this;
}

Framebuffer::Config &
Framebuffer::Config::setResolution(const VkExtent2D &extent) {
  resolution_ = extent;
  return *this;
}

Framebuffer::Config &Framebuffer::Config::setLayers(u32 layers) {
  layers_ = layers;
  return *this;
}

Result<Framebuffer>
Framebuffer::Config::create(VkDevice vk_device,
                            VkRenderPass vk_renderpass) const {
  VkFramebufferCreateInfo info{};

  info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  info.pNext = nullptr;
  info.flags = 0;
  info.renderPass = vk_renderpass;
  info.attachmentCount = static_cast<uint32_t>(attachments_.size());
  info.pAttachments = (!attachments_.empty() ? attachments_.data() : nullptr);
  info.width = resolution_.width;
  info.height = resolution_.height;
  info.layers = layers_;

  Framebuffer framebuffer;
  framebuffer.vk_device_ = vk_device;
  framebuffer.resolution_ = resolution_;

  VENUS_VK_RETURN_BAD_RESULT(vkCreateFramebuffer(vk_device, &info, nullptr,
                                                 &framebuffer.vk_framebuffer_));

  return Result<Framebuffer>(std::move(framebuffer));
}

Framebuffer::Framebuffer(Framebuffer &&rhs) noexcept { *this = std::move(rhs); }

Framebuffer::~Framebuffer() noexcept { destroy(); }

Framebuffer &Framebuffer::operator=(Framebuffer &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void Framebuffer::destroy() noexcept {
  if (vk_device_ && vk_framebuffer_)
    vkDestroyFramebuffer(vk_device_, vk_framebuffer_, nullptr);
  vk_device_ = VK_NULL_HANDLE;
  vk_framebuffer_ = VK_NULL_HANDLE;
}

void Framebuffer::swap(Framebuffer &rhs) noexcept {
  VENUS_SWAP_FIELD_WITH_RHS(vk_device_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_framebuffer_);
  VENUS_SWAP_FIELD_WITH_RHS(resolution_);
}

VkFramebuffer Framebuffer::operator*() const { return vk_framebuffer_; }

Framebuffers::Framebuffers(Framebuffers &&rhs) noexcept {
  *this = std::move(rhs);
}

Framebuffers &Framebuffers::operator=(Framebuffers &&rhs) noexcept {
  destroy();
  this->swap(rhs);
  return *this;
}

Framebuffers::~Framebuffers() noexcept { destroy(); }

void Framebuffers::destroy() noexcept { clear(); }

} // namespace venus::pipeline
