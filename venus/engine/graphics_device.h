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

/// \file   graphics_device.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30
/// \brief  Boilerplate class for using graphics devices.

#pragma once

#include <venus/core/device.h>
#include <venus/core/instance.h>
#include <venus/core/sync.h>
#include <venus/io/swapchain.h>
#include <venus/pipeline/command_buffer.h>
#include <venus/pipeline/framebuffer.h>
#include <venus/pipeline/renderpass.h>

#ifndef VENUS_MAX_SWAPCHAIN_IMAGE_COUNT
#define VENUS_MAX_SWAPCHAIN_IMAGE_COUNT 3
#endif

namespace venus::engine {

/// The Graphics Device is responsible for managing the graphics hardware
/// capable to generate images into frame buffers. This class manages all
/// operations for submitting commands to the GPU and hold the following
/// resources:
///   - Physical and Logical device pair.
///   - Swapchain
///   - Command buffers
///   - Renderpass
/// \note The graphics device creates and holds device instances internally,
///       therefore this class should also be the means to access them.
/// \note The destroy() method should be called manually to ensure vulkan
///       resources release order.
/// \note RAII
class GraphicsDevice {
public:
  struct Config {
    Config &setSurfaceExtent(const VkExtent2D &extent);
    Config &setSurface(VkSurfaceKHR surface);
    Config &setFeatures(const core::vk::DeviceFeatures &features);
    Config &addExtension(const std::string_view &extension);
    Config &addExtensions(const std::vector<std::string> &extensions);

    Result<GraphicsDevice> create(const core::Instance &instance) const;

  private:
    bool useDynamicRendering() const;

    VkExtent2D surface_extent_{};
    VkSurfaceKHR surface_{VK_NULL_HANDLE};
    core::vk::DeviceFeatures features_{};
    std::vector<std::string> extensions_;
  };

  struct Output {
    mem::AllocatedImage color;
    mem::Image::View color_view;
    mem::AllocatedImage depth;
    mem::Image::View depth_view;
  };

  /// \note Although the destructor calls destroy(), the destroy method should
  ///       usually be called manually to ensure vulkan resources release order.
  VENUS_DECLARE_RAII_FUNCTIONS(GraphicsDevice)

  /// Release and clears all internal objects.
  /// \note This method should be called intentionally to follow vulkan
  ///       resources release order.
  VeResult destroy() noexcept;
  void swap(GraphicsDevice &rhs) noexcept;

  // Run commands must be called every frame and on the same order bellow:

  /// \brief Performs pre-frame operations.
  /// \note Acquires swapchain image
  HERMES_NODISCARD VeResult prepare();
  /// \brief Initiates current command buffer
  HERMES_NODISCARD VeResult
  beginRecord(const VkCommandBufferUsageFlags &flags = {}) const;
  /// \brief Initiates renderpass
  void beginRenderPass();
  /// \brief Finishes renderpass
  void endRenderPass();
  /// \brief Finishes current command buffer
  HERMES_NODISCARD VeResult endRecord() const;
  /// \brief Submit the current command buffer record.
  HERMES_NODISCARD VeResult submit() const;
  /// \brief Submit work and present
  HERMES_NODISCARD VeResult finish();

  // Command buffer access

  /// Accesses the current command buffer.
  /// \param f Callback receiving access to the command buffer.
  void
  record(const std::function<void(const pipeline::CommandBuffer &)> &f) const;
  /// Accesses the current command buffer and submit it to the device.
  /// \param f Callback receiving access to the command buffer.
  HERMES_NODISCARD VeResult
  submit(const std::function<void(const pipeline::CommandBuffer &)> &f) const;

  HERMES_NODISCARD VeResult immediateSubmit(
      const std::function<void(const pipeline::CommandBuffer &)> &f) const;

  // Fields access

  /// \return The device pair managed by this object.
  const core::Device &operator*() const;
  /// \return The swapchain managed by this object.
  const io::Swapchain &swapchain() const;
  /// \return The current active command buffer object.
  const pipeline::CommandBuffer &commandBuffer() const;
  /// \return The current swapchain image index.
  u32 currentTargetIndex() const;
  /// \return Output images.
  const Output &output() const;

  // Non-dynamic rendering

  /// \return The renderpass managed by this object.
  const pipeline::RenderPass &renderpass() const;
  /// \return The current active framebuffer.
  const pipeline::Framebuffer &framebuffer() const;

private:
  // devices
  core::Device device_;
  // presentation
  VkSurfaceKHR presentation_surface_;
  VkExtent2D surface_extent_;
  VkQueue presentation_queue_{VK_NULL_HANDLE};
  VkQueue graphics_queue_{VK_NULL_HANDLE};
  // swapchain
  io::Swapchain swapchain_;

  // Non-dynamic rendering

  pipeline::RenderPass renderpass_;
  pipeline::Framebuffers framebuffers_;

  struct FrameResources {
    // command buffers
    pipeline::CommandPool command_pool;
    pipeline::CommandBuffers command_buffers;
    // sync
    core::Semaphore image_acquired_semaphore;
    core::Semaphore render_semaphore;
    core::Fence render_fence;
  };

  struct ImmediateSubmitResources {
    // command buffers
    pipeline::CommandPool command_pool;
    pipeline::CommandBuffers command_buffers;
    // sync
    core::Fence fence;
  };

  /// \return Frame data of the current frame
  const FrameResources &frameData() const;

  FrameResources frames_[VENUS_MAX_SWAPCHAIN_IMAGE_COUNT];
  ImmediateSubmitResources imm_submit_data_;
  Output output_;

  u32 swapchain_image_count_{0};
  u32 current_frame_{0};
  u32 swapchain_image_index_{0};
  bool using_dynamic_rendering_{false};
};

} // namespace venus::engine
