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
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(GraphicsDevice, setFeatures,
                                     const core::vk::DeviceFeatures &,
                                     features_ = value)
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(GraphicsDevice, addExtension,
                                     const std::string_view &,
                                     extensions_.emplace_back(value))
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(
    GraphicsDevice, addExtensions, const std::vector<std::string> &,
    extensions_.insert(extensions_.end(), value.begin(), value.end()))

bool GraphicsDevice::Config::useDynamicRendering() const {
  return features_.v13_f.dynamicRendering;
}

Result<GraphicsDevice>
GraphicsDevice::Config::create(const core::Instance &instance) const {
  GraphicsDevice gd;
  gd.surface_extent_ = surface_extent_;
  gd.presentation_surface_ = surface_;
  gd.using_dynamic_rendering_ = useDynamicRendering();

  // select device

  core::PhysicalDevices physical_devices;
  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(physical_devices,
                                           instance.physicalDevices());
  core::PhysicalDevice physical_device;
  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      physical_device,
      physical_devices.select(
          core::PhysicalDevices::Selector().forGraphics(surface_)));
  HERMES_INFO("\n{}", venus::to_string(physical_devices));

  core::vk::GraphicsQueueFamilyIndices indices;
  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      indices, physical_device.selectGraphicsQueueFamilyIndices(surface_));

  // create logical device

  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      gd.device_,
      core::Device::Config()
          .setFeatures(features_)
          .addAllocationFlags(VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT)
          .addQueueFamily(indices.graphics_queue_family_index, {1.f})
          .addQueueFamily(indices.present_queue_family_index, {1.f})
          .addExtensions(extensions_)
          .create(physical_device));

  // get device queues

  vkGetDeviceQueue(*gd.device_, indices.graphics_queue_family_index, 0,
                   &gd.graphics_queue_);
  vkGetDeviceQueue(*gd.device_, indices.present_queue_family_index, 0,
                   &gd.presentation_queue_);

  // swapchain
  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      gd.swapchain_, io::Swapchain::Config()
                         .setSurface(surface_)
                         .setQueueFamilyIndices(indices)
                         .setExtent(surface_extent_)
                         .addUsageFlags(VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
                         .setPresentMode(VK_PRESENT_MODE_FIFO_KHR)
                         .create(gd.device_));
  gd.surface_extent_ = gd.swapchain_.imageExtent();

  // create frame data
  gd.swapchain_image_count_ = gd.swapchain_.imageCount();

  for (u32 i = 0; i < gd.swapchain_image_count_; ++i) {

    // Create Command Pool

    VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
        gd.frames_[i].command_pool,
        pipeline::CommandPool::Config()
            .addCreateFlags(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
            .setQueueFamilyIndex(indices.graphics_queue_family_index)
            .create(*gd.device_));

    // Create Command Buffer

    VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
        gd.frames_[i].command_buffers, gd.frames_[i].command_pool.allocate(
                                           1, VK_COMMAND_BUFFER_LEVEL_PRIMARY));

    // Create Sync

    VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
        gd.frames_[i].image_acquired_semaphore,
        core::Semaphore::Config().create(*gd.device_));

    VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
        gd.frames_[i].render_semaphore,
        core::Semaphore::Config().create(*gd.device_));

    VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
        gd.frames_[i].render_fence,
        core::Fence::Config()
            .setCreateFlags(VK_FENCE_CREATE_SIGNALED_BIT)
            .create(*gd.device_));
  }

  // Immediate submit data

  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      gd.imm_submit_data_.command_pool,
      pipeline::CommandPool::Config()
          .setQueueFamilyIndex(indices.graphics_queue_family_index)
          .addCreateFlags(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
          .create(*gd.device_));

  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      gd.imm_submit_data_.command_buffers,
      gd.imm_submit_data_.command_pool.allocate(
          1, VK_COMMAND_BUFFER_LEVEL_PRIMARY));

  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      gd.imm_submit_data_.fence,
      core::Fence::Config()
          .setCreateFlags(VK_FENCE_CREATE_SIGNALED_BIT)
          .create(*gd.device_));

  // Create Renderpass
  if (!useDynamicRendering()) {
    VkAttachmentDescription color_att{};
    color_att.format = gd.swapchain_.colorFormat();
    color_att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_att.flags = {};
    color_att.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_att.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    color_att.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_att.samples = VK_SAMPLE_COUNT_1_BIT;

    VkAttachmentDescription depth_att{};
    depth_att.format = gd.swapchain_.depthBuffer().format();
    depth_att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth_att.flags = {};
    depth_att.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_att.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_att.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_att.samples = VK_SAMPLE_COUNT_1_BIT;

    VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
        gd.renderpass_,
        pipeline::RenderPass::Config()
            .addAttachment(color_att)
            .addAttachment(depth_att)
            .addSubpass(
                pipeline::RenderPass::Subpass()
                    .addColorAttachmentRef(
                        0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
                    .addColorAttachmentRef(
                        1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL))
            .create(*gd.device_));

    // framebuffers

    const auto &depth_buffer_view = gd.swapchain_.depthBufferView();
    const auto &image_views = gd.swapchain_.imageViews();
    gd.framebuffers_.reserve(image_views.size());

    for (const auto &image_view : image_views) {
      pipeline::Framebuffer framebuffer;
      VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
          framebuffer, pipeline::Framebuffer::Config()
                           .addAttachment(*image_view)
                           .addAttachment(*depth_buffer_view)
                           .setResolution(gd.swapchain_.imageExtent())
                           .setLayers(1)
                           .create(*gd.device_, *gd.renderpass_));
      gd.framebuffers_.emplace_back(std::move(framebuffer));
    }
  }

  // Output Resources

  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      gd.output_.color,
      mem::AllocatedImage::Config()
          .setImageConfig(mem::Image::Config::defaults(gd.surface_extent_)
                              .setFormat(VK_FORMAT_R16G16B16A16_SFLOAT)
                              .addUsage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
                              .addUsage(VK_IMAGE_USAGE_STORAGE_BIT)
                              .addUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT))
          .setMemoryConfig(
              mem::DeviceMemory::Config().setDeviceLocal().setUsage(
                  VMA_MEMORY_USAGE_GPU_ONLY))
          .create(*gd));

  VkImageSubresourceRange subresource_range;
  subresource_range.baseMipLevel = 0;
  subresource_range.levelCount = 1;
  subresource_range.baseArrayLayer = 0;
  subresource_range.layerCount = 1;
  subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      gd.output_.color_view, mem::Image::View::Config()
                                 .setFormat(gd.output_.color.format())
                                 .setViewType(VK_IMAGE_VIEW_TYPE_2D)
                                 .setSubresourceRange(subresource_range)
                                 .create(gd.output_.color));

  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      gd.output_.depth,
      mem::AllocatedImage::Config()
          .setImageConfig(mem::Image::Config::forDepthBuffer(gd.surface_extent_)
                              .setFormat(VK_FORMAT_D32_SFLOAT))
          .setMemoryConfig(
              mem::DeviceMemory::Config().setDeviceLocal().setUsage(
                  VMA_MEMORY_USAGE_GPU_ONLY))
          .create(*gd));

  subresource_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      gd.output_.depth_view, mem::Image::View::Config()
                                 .setFormat(gd.output_.depth.format())
                                 .setViewType(VK_IMAGE_VIEW_TYPE_2D)
                                 .setSubresourceRange(subresource_range)
                                 .create(gd.output_.depth));

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
}

VeResult GraphicsDevice::destroy() noexcept {
  presentation_surface_ = VK_NULL_HANDLE;
  surface_extent_ = {};
  framebuffers_.clear();
  renderpass_.destroy();
  output_.depth.destroy();
  output_.depth_view.destroy();
  output_.color.destroy();
  output_.color_view.destroy();
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
  swapchain_.destroy();
  device_.destroy();
  presentation_queue_ = VK_NULL_HANDLE;
  graphics_queue_ = VK_NULL_HANDLE;
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

const pipeline::RenderPass &GraphicsDevice::renderpass() const {
  return renderpass_;
}

const pipeline::Framebuffer &GraphicsDevice::framebuffer() const {
  HERMES_ASSERT(!using_dynamic_rendering_);
  HERMES_ASSERT(current_frame_ % swapchain_image_count_ < framebuffers_.size());
  return framebuffers_[current_frame_ % swapchain_image_count_];
}

const GraphicsDevice::FrameResources &GraphicsDevice::frameData() const {
  return frames_[current_frame_ % swapchain_image_count_];
}

VeResult GraphicsDevice::prepare() {
  const auto &frame = frameData();

  vkDeviceWaitIdle(*device_);

  VENUS_VK_RETURN_BAD_RESULT(frame.render_fence.wait());

  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      swapchain_image_index_,
      swapchain_.nextImage(*frame.image_acquired_semaphore));

  VENUS_VK_RETURN_BAD_RESULT(frame.render_fence.reset());

  return VeResult::noError();
}

VeResult GraphicsDevice::submit() const {
  const auto &frame = frameData();

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

  return VeResult::noError();
}

VeResult GraphicsDevice::finish() {
  const auto &frame = frameData();
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
    HERMES_INFO("vk::Queue::presentKHR returned vk::SUCCESS!");
    break;
  case VK_ERROR_OUT_OF_DATE_KHR:
    HERMES_INFO("vk::Queue::presentKHR returned vk::OUT_OF_DATE_KHR!");
    // resize_requested = true;
    break;
  case VK_SUBOPTIMAL_KHR:
    HERMES_INFO("vk::Queue::presentKHR returned vk::Result::eSuboptimalKHR !");
    break;
  default:
    HERMES_ASSERT(false); // an unexpected result is returned !
  }

  // increase the number of frames drawn
  current_frame_++;

  vkQueueWaitIdle(graphics_queue_);

  return VeResult::noError();
}

VeResult
GraphicsDevice::beginRecord(const VkCommandBufferUsageFlags &flags) const {
  const auto &frame = frameData();
  VENUS_RETURN_BAD_RESULT(frame.command_buffers[0].reset({}));
  VENUS_RETURN_BAD_RESULT(frame.command_buffers[0].begin(flags));
  return VeResult::noError();
}

VeResult GraphicsDevice::endRecord() const {
  const auto &frame = frameData();
  VENUS_RETURN_BAD_RESULT(frame.command_buffers[0].end());
  return VeResult::noError();
}

void GraphicsDevice::record(
    const std::function<void(const pipeline::CommandBuffer &)> &f) const {
  const auto &frame = frameData();
  f(frame.command_buffers[0]);
}

VeResult GraphicsDevice::submit(
    const std::function<void(const pipeline::CommandBuffer &)> &f) const {
  const auto &frame = frameData();
  VENUS_RETURN_BAD_RESULT(
      beginRecord(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT));
  f(frame.command_buffers[0]);
  VENUS_RETURN_BAD_RESULT(endRecord());
  VENUS_RETURN_BAD_RESULT(submit());
  return VeResult::noError();
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

} // namespace venus::engine
