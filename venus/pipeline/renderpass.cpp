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

/// \file   renderpass.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30

#include <venus/pipeline/renderpass.h>

#include <venus/utils/vk_debug.h>

namespace venus::pipeline {

RenderPass::Subpass &
RenderPass::Subpass::addInputAttachmentRef(u32 attachment, VkImageLayout layout,
                                           u32 *ref_index) {
  if (ref_index)
    *ref_index = vk_input_attachments_.size();
  VkAttachmentReference ar = {attachment, layout};
  vk_input_attachments_.emplace_back(ar);
  return *this;
}

RenderPass::Subpass &
RenderPass::Subpass::addColorAttachmentRef(u32 attachment, VkImageLayout layout,
                                           u32 *ref_index) {
  if (ref_index)
    *ref_index = vk_color_attachments_.size();
  VkAttachmentReference ar = {attachment, layout};
  vk_color_attachments_.emplace_back(ar);
  return *this;
}

RenderPass::Subpass &RenderPass::Subpass::addResolveAttachmentRef(
    u32 resolve_attachment, VkImageLayout resolve_layout, u32 *ref_index) {
  if (ref_index)
    *ref_index = vk_resolve_attachments_.size();
  VkAttachmentReference rar = {resolve_attachment, resolve_layout};
  vk_resolve_attachments_.emplace_back(rar);
  return *this;
}

RenderPass::Subpass &
RenderPass::Subpass::setDepthStencilAttachmentRef(u32 attachment,
                                                  VkImageLayout layout) {
  vk_depth_stencil_attachment_.attachment = attachment;
  vk_depth_stencil_attachment_.layout = layout;
  depth_stencil_attachment_set_ = true;
  return *this;
}

RenderPass::Subpass &RenderPass::Subpass::preserveAttachment(u32 attachment) {
  preserve_attachments_.emplace_back(attachment);
  return *this;
}

RenderPass::Config &RenderPass::Config::addSubpass(const Subpass &subpass,
                                                   u32 *ref_index) {
  if (ref_index)
    *ref_index = subpasses_.size();
  subpasses_.emplace_back(subpass);
  return *this;
}

RenderPass::Config &
RenderPass::Config::addAttachment(const VkAttachmentDescription &description,
                                  u32 *ref_index) {
  if (ref_index)
    *ref_index = vk_attachments_.size();
  vk_attachments_.emplace_back(description);
  return *this;
}

RenderPass::Config &RenderPass::Config::addSubpassDependency(
    const VkSubpassDependency &subpass_dependency) {
  vk_subpass_dependencies_.emplace_back(subpass_dependency);
  return *this;
}

Result<RenderPass> RenderPass::Config::create(VkDevice vk_device) const {
  std::vector<VkSubpassDescription> subpass_descriptions;
  for (auto &sd : subpasses_) {
    VkSubpassDescription info{};
    info.flags = {};
    info.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    info.inputAttachmentCount =
        static_cast<uint32_t>(sd.vk_input_attachments_.size());
    info.pInputAttachments =
        (!sd.vk_input_attachments_.empty() ? sd.vk_input_attachments_.data()
                                           : nullptr);
    info.colorAttachmentCount =
        static_cast<uint32_t>(sd.vk_color_attachments_.size());
    info.pColorAttachments =
        (!sd.vk_color_attachments_.empty() ? sd.vk_color_attachments_.data()
                                           : nullptr);
    info.pResolveAttachments =
        (!sd.vk_resolve_attachments_.empty() ? sd.vk_resolve_attachments_.data()
                                             : nullptr);
    info.pDepthStencilAttachment =
        (sd.depth_stencil_attachment_set_ ? &sd.vk_depth_stencil_attachment_
                                          : nullptr);
    info.preserveAttachmentCount =
        static_cast<uint32_t>(sd.preserve_attachments_.size());
    info.pPreserveAttachments =
        (!sd.preserve_attachments_.empty() ? sd.preserve_attachments_.data()
                                           : nullptr);
    subpass_descriptions.emplace_back(info);
  }
  VkRenderPassCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  info.pNext = nullptr;
  info.flags = {};
  info.attachmentCount = static_cast<uint32_t>(vk_attachments_.size());
  info.pAttachments = vk_attachments_.data();
  info.subpassCount = static_cast<uint32_t>(subpass_descriptions.size());
  info.pSubpasses = subpass_descriptions.data();
  info.dependencyCount = static_cast<uint32_t>(vk_subpass_dependencies_.size());
  info.pDependencies = vk_subpass_dependencies_.data();

  RenderPass renderpass;
  renderpass.vk_device_ = vk_device;
  VENUS_VK_RETURN_BAD_RESULT(vkCreateRenderPass(vk_device, &info, nullptr,
                                                &renderpass.vk_render_pass_));
  return Result<RenderPass>(std::move(renderpass));
}

RenderPass::RenderPass(RenderPass &&rhs) noexcept { *this = std::move(rhs); }

RenderPass::~RenderPass() noexcept { destroy(); }

RenderPass &RenderPass::operator=(RenderPass &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void RenderPass::swap(RenderPass &rhs) noexcept {
  VENUS_SWAP_FIELD_WITH_RHS(vk_render_pass_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_device_);
}

void RenderPass::destroy() noexcept {
  if (vk_device_ && vk_render_pass_)
    vkDestroyRenderPass(vk_device_, vk_render_pass_, nullptr);
  vk_device_ = VK_NULL_HANDLE;
  vk_render_pass_ = VK_NULL_HANDLE;
}

VkRenderPass RenderPass::operator*() const { return vk_render_pass_; }

} // namespace venus::pipeline
