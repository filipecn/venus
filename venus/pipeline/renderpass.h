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

/// \file   renderpass.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30
/// \brief  Vulkan renderpass.

#pragma once

#include <venus/core/device.h>

namespace venus::pipeline {

/// A single renderpass object encapsulates multiple passes or rendering phases
/// over a single set of output images. Renderpass objects can contain multiple
/// subpasses.
/// Vulkan can figure out which subpasses are dependent on one another. However,
/// there are cases in which dependencies cannot be figured out automatically
/// (for example, when a subpass writes directly to a resource and a subsequent
/// subpass reads data back). Then dependencies must be explicitly defined.
class RenderPass {
public:
  /// A pass within the renderpass is called a subpass. Each subpass references
  /// a number of attachments (from the order added on the RenderPass object) as
  /// input and outputs.
  class Subpass {
  public:
    /// Adds an input attachment reference
    /// Input attachments are attachments from which the subpass can read from.
    /// \param[in] attachment  Index into the array of attachments created in
    ///                        the RenderPass object.
    /// \param[in] layout      Image layout that the attachment is expected to
    ///                        be in at this subpass.
    /// \param[out] ref_index [def=null] Index of the added attachment
    ///                                  reference.
    Subpass &addInputAttachmentRef(u32 attachment, VkImageLayout layout,
                                   u32 *ref_index = nullptr);
    /// Adds an color attachment reference
    /// Color attachments are attachments from which the subpass write to.
    /// Resolve attachments are attachments that correspond to color
    /// attachments, and are used to resove multisample image data
    /// \param[in] attachment Index into the array of attachments created in
    ///                       the RenderPass object.
    /// \param[in] layout     Image layout that the attachment is expected to be
    ///                       in at this subpass.
    /// \param[out] ref_index [def=null] Index of the added attachment
    ///                                  reference.
    Subpass &addColorAttachmentRef(u32 attachment, VkImageLayout layout,
                                   u32 *ref_index = nullptr);
    /// \param[in] resolve_attachment Index into the array of attachments
    ///                               created in the RenderPass object.
    /// \param[in] resolve_layout     Image layout that the attachment is
    ///                               expected to be in at this subpass.
    /// \param[out] ref_index [def=null] Index of the added attachment
    ///                                  reference.
    Subpass &addResolveAttachmentRef(u32 resolve_attachment,
                                     VkImageLayout resolve_layout,
                                     u32 *ref_index = nullptr);
    /// Sets the depth-stencil attachment reference
    /// The depth/stencil attachment is the attachment used as a depth and
    /// stencil buffer.
    /// \param[in] attachment Index into the array of attachments created in
    ///                       the RenderPass object.
    /// \param[in] layout     Image layout that the attachment is expected to be
    ///                       in at this subpass.
    Subpass &setDepthStencilAttachmentRef(u32 attachment, VkImageLayout layout);
    /// Preserved attachments live across the subpass and prevent Vulkan from
    /// making any optimizations that might disturb its contents.
    /// \param attachment Index into the array of attachments created in the
    ///                   RenderPass object
    Subpass &preserveAttachment(u32 attachment);

  private:
    friend class RenderPass;

    bool depth_stencil_attachment_set_{false};
    VkAttachmentReference vk_depth_stencil_attachment_{};
    std::vector<VkAttachmentReference> vk_input_attachments_;
    std::vector<VkAttachmentReference> vk_color_attachments_;
    std::vector<VkAttachmentReference> vk_resolve_attachments_;
    std::vector<u32> preserve_attachments_;
  };
  /// Builder for RenderPass class.
  struct Config {
    /// Adds a subpass description.
    /// \param[out] ref_index [def=null] Subpass index.
    Config &addSubpass(const Subpass &subpass, u32 *ref_index = nullptr);
    /// The load/store ops parameters specify what to do with the attachment at
    /// the beginning and end of the render pass.
    /// \param[in] description Attachment description object.
    /// \param[out] ref_index [def=null] Attachment index.
    Config &addAttachment(const VkAttachmentDescription &description,
                          u32 *ref_index = nullptr);
    /// Defines a dependency between subpasses.
    /// \note Subpass indices are provided by addSubpass method.
    /// \param subpass_dependency
    Config &addSubpassDependency(const VkSubpassDependency &subpass_dependency);

    Result<RenderPass> build(VkDevice vk_device) const;

  private:
    std::vector<VkAttachmentDescription> vk_attachments_;
    std::vector<VkSubpassDependency> vk_subpass_dependencies_;
    std::vector<Subpass> subpasses_;
  };

  VENUS_DECLARE_RAII_FUNCTIONS(RenderPass);

  void destroy() noexcept;
  void swap(RenderPass &rhs) noexcept;
  VkRenderPass operator*() const;

private:
  VkRenderPass vk_render_pass_{VK_NULL_HANDLE};
  VkDevice vk_device_{VK_NULL_HANDLE};
};

} // namespace venus::pipeline
