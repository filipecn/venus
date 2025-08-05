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

/// \file   framebuffer.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30
/// \brief  Vulkan framebuffer.

#pragma once

#include <venus/core/device.h>

namespace venus::pipeline {

/// A framebuffer is an object that represents the set of images that graphics
/// pipelines render into. It is created by using a reference to a renderpass
/// and can be used with any renderpass that has a similar arrangement of
/// attachments.
/// \note This class uses RAII.
class Framebuffer {
public:
  struct Config {
    /// \brief Bounds an image into the framebuffer
    /// The passes comprising the renderpass make references to the image
    /// attachments, and those refrences are specified as indices of the array
    /// constructed from theses additions. In order to make the framebuffer
    /// compatible to a renderpass, you are allowed to add image views with
    /// a null handle (VkNullHandle).
    /// \param image_view
    Config &addAttachment(VkImageView image_view);
    /// \param extent in pixels.
    Config &setResolution(const VkExtent2D &extent);
    /// \param layers
    Config &setLayers(u32 layers);

    Result<Framebuffer> create(VkDevice vk_device,
                               VkRenderPass vk_renderpass) const;

  private:
    u32 layers_{1};
    std::vector<VkImageView> attachments_;
    VkExtent2D resolution_{};
  };

  VENUS_DECLARE_RAII_FUNCTIONS(Framebuffer);

  void destroy() noexcept;
  VkFramebuffer operator*() const;

private:
  VkExtent2D resolution_{};
  VkFramebuffer vk_framebuffer_{VK_NULL_HANDLE};
  VkDevice vk_device_{VK_NULL_HANDLE};
};

class Framebuffers : public std::vector<Framebuffer> {
public:
  ~Framebuffers() noexcept;

  void destroy() noexcept;
};

} // namespace venus::pipeline
