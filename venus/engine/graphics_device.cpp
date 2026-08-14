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

/// \file   graphics_device.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30

#include <venus/engine/graphics_device.h>

#include <venus/utils/vk_debug.h>

namespace venus::engine {

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(GraphicsDevice, setSurfaceExtent,
                                     const VkExtent2D &,
                                     surface_extent_ = value)
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(GraphicsDevice, setSurface, VkSurfaceKHR,
                                     surface_ = value)
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(GraphicsDevice, setDisplay,
                                     io::Display::Ptr, display_ = value)
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(GraphicsDevice, setFeatures,
                                     const core::vk::DeviceFeatures &,
                                     device_features_ = value)
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(GraphicsDevice, addExtension,
                                     const std::string_view &,
                                     extensions_.emplace_back(value))
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(
    GraphicsDevice, addExtensions, const std::vector<std::string> &,
    extensions_.insert(extensions_.end(), value.begin(), value.end()))

bool GraphicsDevice::Config::useDynamicRendering() const {
  return device_features_.v13_f.dynamicRendering;
}

Result<GraphicsDevice>
GraphicsDevice::Config::build(const core::Instance &instance) const {
  GraphicsDevice gd;
  gd.surface_extent_ = surface_extent_;
  gd.presentation_surface_ = surface_;
  gd.using_dynamic_rendering_ = useDynamicRendering();
  gd.display_ = display_;
  if (!display_) {
    return VeResult::inputError();
  }

  // select device

  core::PhysicalDevices physical_devices;
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(physical_devices,
                                    instance.physicalDevices());
  core::PhysicalDevice physical_device;
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      physical_device,
      physical_devices.select(
          core::PhysicalDevices::Selector().forGraphics(surface_)));
  HERMES_INFO("\n{}", VENUS_TO_STRING(physical_devices));

  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      gd.queue_family_indices_,
      physical_device.selectGraphicsQueueFamilyIndices(surface_));

  // create logical device
  auto device_config =
      core::Device::Config()
          .setFeatures(device_features_)
          .addAllocationFlags(VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT)
          .addExtensions(extensions_);

  // add one queue for graphics and another for present if possible
  device_config.addQueueFamily(
      gd.queue_family_indices_.graphics_queue_family_index, {1.f});
  if (gd.queue_family_indices_.graphics_queue_family_index ==
      gd.queue_family_indices_.present_queue_family_index) {
    VENUS_DECLARE_OR_RETURN_BAD_RESULT(
        VkQueueFamilyProperties, qp,
        physical_device.queueFamilyProperties(
            gd.queue_family_indices_.graphics_queue_family_index));
    if (qp.queueCount > 1)
      device_config.addQueueFamily(
          gd.queue_family_indices_.present_queue_family_index, {1.f});
  } else
    device_config.addQueueFamily(
        gd.queue_family_indices_.present_queue_family_index, {1.f});

  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(gd.device_,
                                    device_config.build(physical_device));

  HERMES_INFO("logical device :\n {}", VENUS_TO_STRING(gd.device_));

  // get device queues

  vkGetDeviceQueue(*gd.device_,
                   gd.queue_family_indices_.graphics_queue_family_index, 0,
                   &gd.graphics_queue_);
  vkGetDeviceQueue(*gd.device_,
                   gd.queue_family_indices_.present_queue_family_index, 0,
                   &gd.presentation_queue_);

  // swapchain
  VENUS_RETURN_BAD_RESULT(gd.recreateSwapchain(surface_extent_));

  // create frame data
  for (u32 i = 0; i < VENUS_MAX_SWAPCHAIN_IMAGE_COUNT; ++i) {
    VENUS_RETURN_BAD_RESULT(gd.createFrameResources(i));
  }

  // Immediate submit data

  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      gd.imm_submit_data_.command_pool,
      pipeline::CommandPool::Config()
          .setQueueFamilyIndex(
              gd.queue_family_indices_.graphics_queue_family_index)
          .addCreateFlags(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
          .build(*gd.device_));

  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(gd.imm_submit_data_.command_buffers,
                                    gd.imm_submit_data_.command_pool.allocate(
                                        1, VK_COMMAND_BUFFER_LEVEL_PRIMARY));

  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      gd.imm_submit_data_.fence,
      core::Fence::Config()
          .setCreateFlags(VK_FENCE_CREATE_SIGNALED_BIT)
          .build(*gd.device_));

  VENUS_RETURN_BAD_RESULT(gd.createFramebuffers());
  VENUS_RETURN_BAD_RESULT(gd.createOutputResources());

  return Result<GraphicsDevice>(std::move(gd));
}

GraphicsDevice::GraphicsDevice(GraphicsDevice &&rhs) noexcept {
  *this = std::move(rhs);
}

GraphicsDevice::~GraphicsDevice() noexcept { destroy(); }

GraphicsDevice &GraphicsDevice::operator=(GraphicsDevice &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void GraphicsDevice::swap(GraphicsDevice &rhs) noexcept {
  VENUS_FIELD_SWAP_RHS(device_);
  VENUS_SWAP_FIELD_WITH_RHS(presentation_surface_);
  VENUS_SWAP_FIELD_WITH_RHS(surface_extent_);
  VENUS_SWAP_FIELD_WITH_RHS(presentation_queue_);
  VENUS_SWAP_FIELD_WITH_RHS(graphics_queue_);
  VENUS_FIELD_SWAP_RHS(swapchain_);
  VENUS_SWAP_FIELD_WITH_RHS(swapchain_image_count_);
  VENUS_SWAP_FIELD_WITH_RHS(current_frame_);
  for (int i = 0; i < VENUS_MAX_SWAPCHAIN_IMAGE_COUNT; ++i) {
    VENUS_FIELD_SWAP_RHS(frames_[i].command_pool);
    VENUS_FIELD_SWAP_RHS(frames_[i].command_buffers);
    VENUS_FIELD_SWAP_RHS(frames_[i].image_acquired_semaphore);
    VENUS_FIELD_SWAP_RHS(frames_[i].render_semaphore);
    VENUS_FIELD_SWAP_RHS(frames_[i].render_fence);
  }
  VENUS_FIELD_SWAP_RHS(imm_submit_data_.command_pool);
  VENUS_FIELD_SWAP_RHS(imm_submit_data_.command_buffers);
  VENUS_FIELD_SWAP_RHS(imm_submit_data_.fence);
  VENUS_FIELD_SWAP_RHS(renderpass_);
  VENUS_FIELD_SWAP_RHS(framebuffers_);
  VENUS_FIELD_SWAP_RHS(output_.color_view);
  VENUS_FIELD_SWAP_RHS(output_.color);
  VENUS_FIELD_SWAP_RHS(output_.depth_view);
  VENUS_FIELD_SWAP_RHS(output_.depth);
  std::swap(display_, rhs.display_);
}

VeResult GraphicsDevice::destroy() noexcept {
  presentation_surface_ = VK_NULL_HANDLE;
  surface_extent_ = {};
  imm_submit_data_.fence.destroy();
  imm_submit_data_.command_buffers.clear();
  imm_submit_data_.command_pool.destroy();
  for (u32 i = 0; i < VENUS_MAX_SWAPCHAIN_IMAGE_COUNT; ++i) {
    frames_[i].image_acquired_semaphore.destroy();
    frames_[i].render_semaphore.destroy();
    frames_[i].render_fence.destroy();
    frames_[i].command_buffers.clear();
    frames_[i].command_pool.destroy();
  }
  destroySwapchainDependentResources();
  swapchain_.destroy();
  device_.destroy();
  presentation_queue_ = VK_NULL_HANDLE;
  graphics_queue_ = VK_NULL_HANDLE;
  display_.destroy();
  return VeResult::noError();
}

const core::Device &GraphicsDevice::operator*() const { return device_; }

const io::Swapchain &GraphicsDevice::swapchain() const { return swapchain_; }

const pipeline::CommandBuffer &GraphicsDevice::commandBuffer() const {
  return frameData().command_buffers[0];
}

u32 GraphicsDevice::currentTargetIndex() const {
  return swapchain_image_index_;
}

const GraphicsDevice::Output &GraphicsDevice::output() const { return output_; }

VkQueue GraphicsDevice::graphicsQueue() const { return graphics_queue_; }

const pipeline::RenderPass &GraphicsDevice::renderpass() const {
  return renderpass_;
}

const pipeline::Framebuffer &GraphicsDevice::framebuffer() const {
  HERMES_ASSERT(!using_dynamic_rendering_);
  HERMES_ASSERT((current_frame_ % swapchain_image_count_) <
                framebuffers_.size());
  return framebuffers_[current_frame_ % swapchain_image_count_];
}

const GraphicsDevice::FrameResources &GraphicsDevice::frameData() const {
  return frames_[current_frame_ % swapchain_image_count_];
}

VeResult GraphicsDevice::begin(const VkCommandBufferUsageFlags &flags) {

  vkDeviceWaitIdle(*device_);

  // acquire image

  VENUS_VK_RETURN_BAD_RESULT(frameData().render_fence.wait());

  if (auto res = swapchain_.nextImage(*frameData().image_acquired_semaphore)) {
    swapchain_image_index_ = std::move(*res);
  } else if (res.status() == VeResult(VeResult::Type::SwapchainNeedsResize)) {
    auto new_size = display_->waitSize();
    // recreate swapchain
    VENUS_RETURN_BAD_RESULT(recreateSwapchain(new_size));
    return res.status();
  } else {
    return res.status();
  }

  const auto &frame = frameData();

  VENUS_VK_RETURN_BAD_RESULT(frame.render_fence.reset());

  // begin record

  VENUS_RETURN_BAD_RESULT(frame.command_buffers[0].reset({}));
  VENUS_RETURN_BAD_RESULT(frame.command_buffers[0].begin(flags));

  return VeResult::noError();
}

VeResult GraphicsDevice::finish() {
  const auto &frame = frameData();

  // prepare image for rendering

  VkImage image = *swapchain_.images()[swapchain_image_index_];
  frame.command_buffers[0].transitionImage(
      image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

  // end command buffer record

  VENUS_RETURN_BAD_RESULT(frame.command_buffers[0].end());

  // prepare the submission to the queue.
  // we want to wait on the present semaphore, as that semaphore is signaled
  // when the swapchain is ready we will signal the render semaphore, to signal
  // that rendering has finished

  VENUS_VK_RETURN_BAD_RESULT(
      pipeline::SubmitInfo2()
          .addWaitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
                       *frame.image_acquired_semaphore)
          .addSignalInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
                         *frame.render_semaphore)
          .addCommandBufferInfo(*frame.command_buffers[0])
          .submit(graphics_queue_, *frame.render_fence));

  // prepare present

  //  this will put the image we just rendered to into the visible window.
  //  we want to wait on the render semaphore for that,
  //  as its necessary that drawing commands have finished before the image is
  //  displayed to the user
  auto wait_semaphores = *frame.render_semaphore;

  auto swapchain = *swapchain_;

  VkPresentInfoKHR present_info{};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.pNext = nullptr;
  present_info.swapchainCount = 1;
  present_info.waitSemaphoreCount = 1;
  present_info.pImageIndices = &swapchain_image_index_;
  present_info.pWaitSemaphores = &wait_semaphores;
  present_info.pSwapchains = &swapchain;
  present_info.pResults = nullptr;

  VkResult result = vkQueuePresentKHR(graphics_queue_, &present_info);
  switch (result) {
  case VK_SUCCESS:
    break;
  case VK_ERROR_OUT_OF_DATE_KHR:
  case VK_SUBOPTIMAL_KHR:
    // recreate swapchain
    VENUS_RETURN_BAD_RESULT(recreateSwapchain(display_->waitSize()));
    break;
  default:
    HERMES_ASSERT(false); // an unexpected result is returned !
  }

  // increase the number of frames displayed
  current_frame_++;

  return VeResult::noError();
}

void GraphicsDevice::record(
    const std::function<void(const pipeline::CommandBuffer &)> &f) const {
  const auto &frame = frameData();
  f(frame.command_buffers[0]);
}

VeResult GraphicsDevice::immediateSubmit(
    const std::function<void(const pipeline::CommandBuffer &)> &f) const {

  VENUS_VK_RETURN_BAD_RESULT(imm_submit_data_.fence.reset());
  const auto &cb = imm_submit_data_.command_buffers[0];

  VENUS_RETURN_BAD_RESULT(cb.reset());
  VENUS_RETURN_BAD_RESULT(
      cb.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT));
  f(cb);
  VENUS_RETURN_BAD_RESULT(cb.end());

  VENUS_VK_RETURN_BAD_RESULT(
      pipeline::SubmitInfo2().addCommandBufferInfo(*cb).submit(
          graphics_queue_, *imm_submit_data_.fence));

  VENUS_VK_RETURN_BAD_RESULT(imm_submit_data_.fence.wait());

  return VeResult::noError();
}

VeResult GraphicsDevice::createFrameResources(h_index i) {
  // Create Command Pool

  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      frames_[i].command_pool,
      pipeline::CommandPool::Config()
          .addCreateFlags(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
          .setQueueFamilyIndex(
              queue_family_indices_.graphics_queue_family_index)
          .build(*device_));

  // Create Command Buffer

  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      frames_[i].command_buffers,
      frames_[i].command_pool.allocate(1, VK_COMMAND_BUFFER_LEVEL_PRIMARY));

  // Create Sync

  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(frames_[i].image_acquired_semaphore,
                                    core::Semaphore::Config().build(*device_));

  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(frames_[i].render_semaphore,
                                    core::Semaphore::Config().build(*device_));

  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      frames_[i].render_fence, core::Fence::Config()
                                   .setCreateFlags(VK_FENCE_CREATE_SIGNALED_BIT)
                                   .build(*device_));

  return VeResult::noError();
}

VeResult GraphicsDevice::createFramebuffers() {
  // Create Renderpass
  if (!using_dynamic_rendering_) {
    VkAttachmentDescription color_att{};
    color_att.format = swapchain_.colorFormat();
    color_att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_att.flags = {};
    color_att.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_att.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    color_att.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_att.samples = VK_SAMPLE_COUNT_1_BIT;

    VkAttachmentDescription depth_att{};
    depth_att.format = swapchain_.depthBuffer().format();
    depth_att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth_att.flags = {};
    depth_att.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_att.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_att.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_att.samples = VK_SAMPLE_COUNT_1_BIT;

    VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
        renderpass_,
        pipeline::RenderPass::Config()
            .addAttachment(color_att)
            .addAttachment(depth_att)
            .addSubpass(
                pipeline::RenderPass::Subpass()
                    .addColorAttachmentRef(
                        0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
                    .addColorAttachmentRef(
                        1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL))
            .build(*device_));

    // framebuffers

    const auto &depth_buffer_view = swapchain_.depthBufferView();
    const auto &image_views = swapchain_.imageViews();
    framebuffers_.reserve(image_views.size());

    for (const auto &image_view : image_views) {
      pipeline::Framebuffer framebuffer;
      VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
          framebuffer, pipeline::Framebuffer::Config()
                           .addAttachment(*image_view)
                           .addAttachment(*depth_buffer_view)
                           .setResolution(swapchain_.imageExtent())
                           .setLayers(1)
                           .build(*device_, *renderpass_));
      framebuffers_.emplace_back(std::move(framebuffer));
    }
  }
  return VeResult::noError();
}

VeResult GraphicsDevice::createOutputResources() {
  // Output Resources
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      output_.color,
      mem::AllocatedImage::Config::forColorAttachment(surface_extent_)
          .build(device_));

  VkImageSubresourceRange subresource_range;
  subresource_range.baseMipLevel = 0;
  subresource_range.levelCount = 1;
  subresource_range.baseArrayLayer = 0;
  subresource_range.layerCount = 1;
  subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(output_.color_view,
                                    mem::Image::View::Config()
                                        .setFormat(output_.color.format())
                                        .setViewType(VK_IMAGE_VIEW_TYPE_2D)
                                        .setSubresourceRange(subresource_range)
                                        .build(output_.color));

  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      output_.depth,
      mem::AllocatedImage::Config::forDepthBuffer(surface_extent_)
          .build(device_));

  subresource_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(output_.depth_view,
                                    mem::Image::View::Config()
                                        .setFormat(output_.depth.format())
                                        .setViewType(VK_IMAGE_VIEW_TYPE_2D)
                                        .setSubresourceRange(subresource_range)
                                        .build(output_.depth));
  return VeResult::noError();
}

VeResult GraphicsDevice::recreateSwapchain(VkExtent2D new_resolution) {
  vkDeviceWaitIdle(*device_);

  // TODO: should we recreate if new_resolution is the same?
  surface_extent_ = new_resolution;

  VENUS_RETURN_BAD_RESULT(destroySwapchainDependentResources());
  VENUS_RETURN_BAD_RESULT(createSwapchainDependentResources());

  io::Swapchain new_swapchain;
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      new_swapchain, io::Swapchain::Config()
                         .setSurface(presentation_surface_)
                         .setOldSwapchain(*swapchain_)
                         .setQueueFamilyIndices(queue_family_indices_)
                         .setExtent(surface_extent_)
                         .addUsageFlags(VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
                         .setPresentMode(VK_PRESENT_MODE_FIFO_KHR)
                         .build(device_));

  swapchain_ = std::move(new_swapchain);

  surface_extent_ = swapchain_.imageExtent();
  swapchain_image_count_ = swapchain_.imageCount();

  return VeResult::noError();
}

VeResult GraphicsDevice::destroySwapchainDependentResources() {
  framebuffers_.clear();
  renderpass_.destroy();
  output_.depth.destroy();
  output_.depth_view.destroy();
  output_.color.destroy();
  output_.color_view.destroy();
  return VeResult::noError();
}

VeResult GraphicsDevice::createSwapchainDependentResources() {
  return VeResult::noError();
}

} // namespace venus::engine
