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

/// \file   scene.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-06-07

#include <venus/app/renderer.h>

#include <venus/engine/graphics_engine.h>

namespace venus::app {

Renderer::~Renderer() noexcept { destroy(); }

void Renderer::destroy() noexcept {}

VeResult Renderer::begin() {
  auto &gd = venus::engine::GraphicsEngine::device();
  auto &cb = gd.commandBuffer();

  VkImage image = *gd.swapchain().images()[gd.currentTargetIndex()];
  VkImageView image_view =
      *gd.swapchain().imageViews()[gd.currentTargetIndex()];
  VkImageView depth_view = *gd.swapchain().depthBufferView();

  // clear screen

  VkClearColorValue clearColor = {30.0f / 256.0f, 30.0f / 256.0f,
                                  134.0f / 256.0f, 0.0f};
  VkClearValue clearValue = {};
  clearValue.color = clearColor;
  VkImageSubresourceRange imageRange = {};
  imageRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  imageRange.levelCount = 1;
  imageRange.layerCount = 1;
  std::vector<VkImageSubresourceRange> ranges;
  ranges.emplace_back(imageRange);

  cb.transitionImage(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

  cb.clear(*venus::engine::GraphicsEngine::device().swapchain().images()[0],
           VK_IMAGE_LAYOUT_GENERAL, ranges, clearColor);

  cb.transitionImage(image, VK_IMAGE_LAYOUT_GENERAL,
                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

  VkClearValue depth_clear;
  depth_clear.depthStencil.depth = 0.f;
  auto rendering_info =
      venus::pipeline::CommandBuffer::RenderingInfo()
          .setLayerCount(1)
          .setRenderArea({VkOffset2D{0, 0}, gd.swapchain().imageExtent()})
          .addColorAttachment(
              venus::pipeline::CommandBuffer::RenderingInfo::Attachment()
                  .setImageLayout(VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL)
                  .setImageView(image_view)
                  .setStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
                  .setLoadOp(VK_ATTACHMENT_LOAD_OP_LOAD))
          .setDepthAttachment(
              venus::pipeline::CommandBuffer::RenderingInfo::Attachment()
                  .setImageLayout(VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
                  .setImageView(depth_view)
                  .setStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
                  .setLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
                  .setClearValue(depth_clear));

  cb.beginRendering(*rendering_info);

  return VeResult::noError();
}

VeResult Renderer::end() {
  auto &gd = venus::engine::GraphicsEngine::device();
  auto &cb = gd.commandBuffer();

  cb.endRendering();

  VkImage image = *gd.swapchain().images()[gd.currentTargetIndex()];
  cb.transitionImage(image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                     VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

  return VeResult::noError();
}

} // namespace venus::app
