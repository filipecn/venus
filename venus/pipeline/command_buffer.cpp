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

/// \file   command_buffer.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30

#include <venus/pipeline/command_buffer.h>

#include <venus/core/sync.h>
#include <venus/engine/graphics_device.h>
#include <venus/utils/vk_debug.h>

namespace venus::pipeline {

CommandBuffer::RenderingInfo::Attachment::Attachment() noexcept {
  info_.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
  info_.pNext = nullptr;
}

VENUS_DEFINE_SET_INFO_FIELD_METHOD(CommandBuffer::RenderingInfo::Attachment,
                                   setImageView, VkImageView, imageView)
VENUS_DEFINE_SET_INFO_FIELD_METHOD(CommandBuffer::RenderingInfo::Attachment,
                                   setImageLayout, VkImageLayout, imageLayout)
VENUS_DEFINE_SET_INFO_FIELD_METHOD(CommandBuffer::RenderingInfo::Attachment,
                                   setResolveMode, VkResolveModeFlagBits,
                                   resolveMode)
VENUS_DEFINE_SET_INFO_FIELD_METHOD(CommandBuffer::RenderingInfo::Attachment,
                                   setResolveImageView, VkImageView,
                                   resolveImageView)
VENUS_DEFINE_SET_INFO_FIELD_METHOD(CommandBuffer::RenderingInfo::Attachment,
                                   setResolveImageLayout, VkImageLayout,
                                   resolveImageLayout)
VENUS_DEFINE_SET_INFO_FIELD_METHOD(CommandBuffer::RenderingInfo::Attachment,
                                   setLoadOp, VkAttachmentLoadOp, loadOp)
VENUS_DEFINE_SET_INFO_FIELD_METHOD(CommandBuffer::RenderingInfo::Attachment,
                                   setStoreOp, VkAttachmentStoreOp, storeOp)
VENUS_DEFINE_SET_INFO_FIELD_METHOD(CommandBuffer::RenderingInfo::Attachment,
                                   setClearValue, VkClearValue, clearValue)

VkRenderingAttachmentInfo
CommandBuffer::RenderingInfo::Attachment::operator*() const {
  return info_;
}

CommandBuffer::RenderingInfo::RenderingInfo() noexcept {
  info_.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
  info_.pNext = nullptr;
}

VENUS_DEFINE_SET_INFO_FIELD_METHOD(CommandBuffer::RenderingInfo, setFlags,
                                   VkRenderingFlags, flags)
VENUS_DEFINE_SET_INFO_FIELD_METHOD(CommandBuffer::RenderingInfo, setRenderArea,
                                   const VkRect2D &, renderArea)
VENUS_DEFINE_SET_INFO_FIELD_METHOD(CommandBuffer::RenderingInfo, setLayerCount,
                                   u32, layerCount)
VENUS_DEFINE_SET_INFO_FIELD_METHOD(CommandBuffer::RenderingInfo, setViewMask,
                                   u32, viewMask)
VENUS_DEFINE_SET_FIELD_METHOD(CommandBuffer::RenderingInfo, addColorAttachment,
                              const CommandBuffer::RenderingInfo::Attachment &,
                              color_attachments_.emplace_back(*value))
VENUS_DEFINE_SET_FIELD_METHOD(CommandBuffer::RenderingInfo, setDepthAttachment,
                              const CommandBuffer::RenderingInfo::Attachment &,
                              depth_attachment_ = *value)
VENUS_DEFINE_SET_FIELD_METHOD(CommandBuffer::RenderingInfo,
                              setStencilAttachment,
                              const CommandBuffer::RenderingInfo::Attachment &,
                              stencil_attachment_ = *value)

VkRenderingInfo CommandBuffer::RenderingInfo::operator*() const {
  VkRenderingInfo info = info_;
  info.colorAttachmentCount = color_attachments_.size();
  info.pColorAttachments = color_attachments_.data();
  info.pDepthAttachment =
      depth_attachment_.has_value() ? &(*depth_attachment_) : nullptr;
  info.pStencilAttachment =
      stencil_attachment_.has_value() ? &(*stencil_attachment_) : nullptr;
  return info;
}

CommandBuffer::RenderPassInfo::RenderPassInfo() noexcept {
  info_.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  info_.pNext = nullptr;
}

CommandBuffer::RenderPassInfo &
CommandBuffer::RenderPassInfo::setRenderArea(i32 x, i32 y, u32 width,
                                             u32 height) {
  info_.renderArea.offset.x = x;
  info_.renderArea.offset.y = y;
  info_.renderArea.extent.width = width;
  info_.renderArea.extent.height = height;
  return *this;
}

CommandBuffer::RenderPassInfo &
CommandBuffer::RenderPassInfo::addClearColorValuef(f32 r, f32 g, f32 b, f32 a) {
  VkClearValue v;
  v.color.float32[0] = r;
  v.color.float32[1] = g;
  v.color.float32[2] = b;
  v.color.float32[3] = a;
  clear_values_.emplace_back(v);
  info_.clearValueCount = clear_values_.size();
  info_.pClearValues = clear_values_.data();
  return *this;
}

CommandBuffer::RenderPassInfo &
CommandBuffer::RenderPassInfo::addClearColorValuei(i32 r, i32 g, i32 b, i32 a) {
  VkClearValue v{};
  v.color.int32[0] = r;
  v.color.int32[1] = g;
  v.color.int32[2] = b;
  v.color.int32[3] = a;
  clear_values_.emplace_back(v);
  info_.clearValueCount = clear_values_.size();
  info_.pClearValues = clear_values_.data();
  return *this;
}

CommandBuffer::RenderPassInfo &
CommandBuffer::RenderPassInfo::addClearColorValueu(u32 r, u32 g, u32 b, u32 a) {
  VkClearValue v{};
  v.color.uint32[0] = r;
  v.color.uint32[1] = g;
  v.color.uint32[2] = b;
  v.color.uint32[3] = a;
  clear_values_.emplace_back(v);
  info_.clearValueCount = clear_values_.size();
  info_.pClearValues = clear_values_.data();
  return *this;
}

CommandBuffer::RenderPassInfo &
CommandBuffer::RenderPassInfo::addClearDepthStencilValue(f32 depth,
                                                         u32 stencil) {
  VkClearValue v{};
  v.depthStencil.depth = depth;
  v.depthStencil.stencil = stencil;
  clear_values_.emplace_back(v);
  info_.clearValueCount = clear_values_.size();
  info_.pClearValues = clear_values_.data();
  return *this;
}

VkRenderPassBeginInfo
CommandBuffer::RenderPassInfo::info(VkRenderPass vk_renderpass,
                                    VkFramebuffer vk_framebuffer) const {
  auto _info = info_;
  _info.renderPass = vk_renderpass;
  _info.framebuffer = vk_framebuffer;
  return _info;
}

CommandBuffer::CommandBuffer(CommandBuffer &&rhs) noexcept {
  *this = std::move(rhs);
}

CommandBuffer::~CommandBuffer() noexcept { destroy(); }

CommandBuffer &CommandBuffer::operator=(CommandBuffer &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void CommandBuffer::swap(CommandBuffer &rhs) noexcept {
  VENUS_SWAP_FIELD_WITH_RHS(vk_command_buffer_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_command_pool_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_device_);
}

void CommandBuffer::destroy() noexcept {
  if (vk_device_ && vk_command_pool_ && vk_command_buffer_)
    vkFreeCommandBuffers(vk_device_, vk_command_pool_, 1, &vk_command_buffer_);
  vk_command_buffer_ = VK_NULL_HANDLE;
  vk_command_pool_ = VK_NULL_HANDLE;
  vk_device_ = VK_NULL_HANDLE;
}

VkCommandBuffer CommandBuffer::operator*() const { return vk_command_buffer_; }

VeResult CommandBuffer::begin(VkCommandBufferUsageFlags flags) const {
  VkCommandBufferBeginInfo info;
  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  info.pNext = nullptr;
  info.flags = flags;
  info.pInheritanceInfo = nullptr;
  VENUS_VK_RETURN_BAD_RESULT(vkBeginCommandBuffer(vk_command_buffer_, &info));
  return VeResult::noError();
}

VeResult CommandBuffer::end() const {
  VENUS_VK_RETURN_BAD_RESULT(vkEndCommandBuffer(vk_command_buffer_));
  return VeResult::noError();
}

VeResult CommandBuffer::reset(VkCommandBufferResetFlags flags) const {
  VENUS_VK_RETURN_BAD_RESULT(vkResetCommandBuffer(vk_command_buffer_, flags));
  return VeResult::noError();
}

VeResult CommandBuffer::submit(VkQueue queue, VkFence fence) const {
  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &vk_command_buffer_;
  vkQueueSubmit(queue, 1, &submit_info, fence);
  return VeResult::noError();
}

void CommandBuffer::copy(VkBuffer vk_src_buffer, VkBuffer vk_dst_buffer,
                         const std::vector<VkBufferCopy> &regions) const {
  vkCmdCopyBuffer(vk_command_buffer_, vk_src_buffer, vk_dst_buffer,
                  regions.size(), regions.data());
}

void CommandBuffer::copy(VkBuffer vk_src_buffer, VkBuffer vk_dst_buffer,
                         VkBufferCopy vk_copy_region) const {
  vkCmdCopyBuffer(vk_command_buffer_, vk_src_buffer, vk_dst_buffer, 1,
                  &vk_copy_region);
}

void CommandBuffer::copy(VkBuffer src_buffer, VkImage dst_image,
                         VkImageLayout layout,
                         const std::vector<VkBufferImageCopy> &regions) const {
  vkCmdCopyBufferToImage(vk_command_buffer_, src_buffer, dst_image, layout,
                         regions.size(), regions.data());
}

void CommandBuffer::copy(VkImage src_image, VkImageLayout layout,
                         VkBuffer dst_buffer,
                         const std::vector<VkBufferImageCopy> &regions) const {
  vkCmdCopyImageToBuffer(vk_command_buffer_, src_image, layout, dst_buffer,
                         regions.size(), regions.data());
}

void CommandBuffer::copy(VkImage src_image, VkImageLayout src_layout,
                         VkImage dst_image, VkImageLayout dst_layout,
                         const std::vector<VkImageCopy> &regions) const {
  vkCmdCopyImage(vk_command_buffer_, src_image, src_layout, dst_image,
                 dst_layout, regions.size(), regions.data());
}

void CommandBuffer::clear(VkImage image, VkImageLayout layout,
                          const std::vector<VkImageSubresourceRange> &ranges,
                          const VkClearColorValue &color) const {
  vkCmdClearColorImage(vk_command_buffer_, image, layout, &color, ranges.size(),
                       ranges.data());
}

void CommandBuffer::clear(VkImage image, VkImageLayout layout,
                          const std::vector<VkImageSubresourceRange> &ranges,
                          const VkClearDepthStencilValue &value) const {
  vkCmdClearDepthStencilImage(vk_command_buffer_, image, layout, &value,
                              ranges.size(), ranges.data());
}

void CommandBuffer::bind(const ComputePipeline &compute_pipeline) const {
  vkCmdBindPipeline(vk_command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE,
                    *compute_pipeline);
}

void CommandBuffer::bind(const GraphicsPipeline &graphics_pipeline) const {
  vkCmdBindPipeline(vk_command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    *graphics_pipeline);
}

void CommandBuffer::bind(VkPipelineBindPoint pipeline_bind_point,
                         VkPipelineLayout layout, u32 first_set,
                         const std::vector<VkDescriptorSet> &descriptor_sets,
                         const std::vector<u32> &dynamic_offsets) const {
  vkCmdBindDescriptorSets(
      vk_command_buffer_, pipeline_bind_point, layout, first_set,
      descriptor_sets.size(), descriptor_sets.data(), dynamic_offsets.size(),
      (dynamic_offsets.size()) ? dynamic_offsets.data() : nullptr);
}

void CommandBuffer::bind(VkPipelineBindPoint pipeline_bind_point,
                         VkPipelineLayout pipeline_layout,
                         const std::vector<VkDescriptorSet> &descriptor_sets,
                         const std::vector<u32> &dynamic_offsets, u32 first_set,
                         u32 descriptor_set_count) const {
  if (!descriptor_set_count)
    descriptor_set_count = descriptor_sets.size() - first_set;
  vkCmdBindDescriptorSets(vk_command_buffer_, pipeline_bind_point,
                          pipeline_layout, first_set, descriptor_set_count,
                          descriptor_sets.data(), dynamic_offsets.size(),
                          dynamic_offsets.data());
}

void CommandBuffer::dispatch(u32 x, u32 y, u32 z) const {
  vkCmdDispatch(vk_command_buffer_, x, y, z);
}

void CommandBuffer::dispatch(VkBuffer buffer, VkDeviceSize offset) const {
  vkCmdDispatchIndirect(vk_command_buffer_, buffer, offset);
}

void CommandBuffer::pushConstants(VkPipelineLayout pipeline_layout,
                                  VkShaderStageFlags stage_flags, u32 offset,
                                  u32 size, const void *values) const {
  vkCmdPushConstants(vk_command_buffer_, pipeline_layout, stage_flags, offset,
                     size, values);
}

void CommandBuffer::beginRenderPass(const VkRenderPassBeginInfo &info,
                                    VkSubpassContents contents) const {
  vkCmdBeginRenderPass(vk_command_buffer_, &info, contents);
}

void CommandBuffer::endRenderPass() const {
  vkCmdEndRenderPass(vk_command_buffer_);
}

void CommandBuffer::beginRendering(const VkRenderingInfo &info) const {
  vkCmdBeginRendering(vk_command_buffer_, &info);
}

void CommandBuffer::endRendering() const {
  vkCmdEndRendering(vk_command_buffer_);
}

void CommandBuffer::bindVertexBuffers(
    u32 first_binding, const std::vector<VkBuffer> &buffers,
    const std::vector<VkDeviceSize> &offsets) const {
  vkCmdBindVertexBuffers(vk_command_buffer_, first_binding, buffers.size(),
                         buffers.data(), offsets.data());
}

void CommandBuffer::bindIndexBuffer(VkBuffer buffer, VkDeviceSize offset,
                                    VkIndexType type) const {
  vkCmdBindIndexBuffer(vk_command_buffer_, buffer, offset, type);
}

void CommandBuffer::draw(u32 vertex_count, u32 instance_count, u32 first_vertex,
                         u32 first_instance) const {
  vkCmdDraw(vk_command_buffer_, vertex_count, instance_count, first_vertex,
            first_instance);
}

void CommandBuffer::drawIndexed(u32 index_count, u32 instance_count,
                                u32 first_index, i32 vertex_offset,
                                u32 first_instance) const {
  vkCmdDrawIndexed(vk_command_buffer_, index_count, instance_count, first_index,
                   vertex_offset, first_instance);
}

void CommandBuffer::transitionImageLayout(
    VkImageMemoryBarrier barrier, VkPipelineStageFlags src_stages,
    VkPipelineStageFlags dst_stages) const {
  vkCmdPipelineBarrier(vk_command_buffer_, src_stages, dst_stages, 0, 0,
                       nullptr, 0, nullptr, 1, &barrier);
}

void CommandBuffer::blit(VkImage src_image, VkImageLayout src_image_layout,
                         VkImage dst_image, VkImageLayout dst_image_layout,
                         const std::vector<VkImageBlit> &regions,
                         VkFilter filter) const {
  vkCmdBlitImage(vk_command_buffer_, src_image, src_image_layout, dst_image,
                 dst_image_layout, regions.size(), &regions[0], filter);
}

void CommandBuffer::setViewport(f32 width, f32 height, f32 min_depth,
                                f32 max_depth) const {
  VkViewport viewport{};
  viewport.width = width;
  viewport.height = height;
  viewport.minDepth = min_depth;
  viewport.maxDepth = max_depth;
  vkCmdSetViewport(vk_command_buffer_, 0, 1, &viewport);
}

void CommandBuffer::setScissor(i32 offset_x, i32 offset_y, u32 extent_width,
                               u32 extent_height) const {
  VkRect2D scissor_rect{};
  scissor_rect.offset.x = offset_x;
  scissor_rect.offset.y = offset_y;
  scissor_rect.extent.width = extent_width;
  scissor_rect.extent.height = extent_height;
  vkCmdSetScissor(vk_command_buffer_, 0, 1, &scissor_rect);
}

void CommandBuffer::transitionImage(VkImage vk_image,
                                    VkImageLayout current_layout,
                                    VkImageLayout new_layout) const {
  VkImageMemoryBarrier2 image_barrier{};
  image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
  image_barrier.pNext = nullptr;

  image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
  image_barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
  image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
  image_barrier.dstAccessMask =
      VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

  image_barrier.oldLayout = current_layout;
  image_barrier.newLayout = new_layout;

  VkImageAspectFlags aspect_mask =
      (new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
          ? VK_IMAGE_ASPECT_DEPTH_BIT
          : VK_IMAGE_ASPECT_COLOR_BIT;

  VkImageSubresourceRange sub_image{};
  sub_image.aspectMask = aspect_mask;
  sub_image.baseMipLevel = 0;
  sub_image.levelCount = VK_REMAINING_MIP_LEVELS;
  sub_image.baseArrayLayer = 0;
  sub_image.layerCount = VK_REMAINING_ARRAY_LAYERS;

  image_barrier.subresourceRange = sub_image;
  image_barrier.image = vk_image;

  VkDependencyInfo dep_info{};
  dep_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
  dep_info.pNext = nullptr;

  dep_info.imageMemoryBarrierCount = 1;
  dep_info.pImageMemoryBarriers = &image_barrier;

  vkCmdPipelineBarrier2(vk_command_buffer_, &dep_info);
}

CommandPool::CommandPool(CommandPool &&rhs) noexcept { *this = std::move(rhs); }

CommandPool::~CommandPool() noexcept { destroy(); }

CommandPool &CommandPool::operator=(CommandPool &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void CommandPool::swap(CommandPool &rhs) noexcept {
  VENUS_SWAP_FIELD_WITH_RHS(vk_command_pool_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_device_);
}

void CommandPool::destroy() noexcept {
  if (vk_device_ && vk_command_pool_)
    vkDestroyCommandPool(vk_device_, vk_command_pool_, nullptr);
  vk_command_pool_ = VK_NULL_HANDLE;
  vk_device_ = VK_NULL_HANDLE;
}

VkCommandPool CommandPool::operator*() const { return vk_command_pool_; }

CommandPool::Config &
CommandPool::Config::addCreateFlags(VkCommandPoolCreateFlagBits flags) {
  flags_ |= flags;
  return *this;
}

CommandPool::Config &
CommandPool::Config::setQueueFamilyIndex(u32 queue_family_index) {
  queue_family_index_ = queue_family_index;
  return *this;
}

Result<CommandPool> CommandPool::Config::create(VkDevice vk_device) const {
  VkCommandPoolCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  info.pNext = nullptr;
  info.flags = flags_;
  info.queueFamilyIndex = queue_family_index_;
  CommandPool command_pool;
  command_pool.vk_device_ = vk_device;
  VENUS_VK_RETURN_BAD_RESULT(vkCreateCommandPool(
      vk_device, &info, nullptr, &command_pool.vk_command_pool_));
  return Result<CommandPool>(std::move(command_pool));
}

VeResult CommandPool::reset(VkCommandPoolResetFlags flags) const {
  VENUS_VK_RETURN_BAD_RESULT(
      vkResetCommandPool(vk_device_, vk_command_pool_, flags));
  return VeResult::noError();
}

Result<CommandBuffer> CommandPool::allocate(VkCommandBufferLevel level) {
  VkCommandBufferAllocateInfo info;
  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  info.pNext = nullptr;
  info.commandPool = vk_command_pool_;
  info.level = level;
  info.commandBufferCount = 1;
  CommandBuffer command_buffer;
  VENUS_VK_RETURN_BAD_RESULT(vkAllocateCommandBuffers(
      vk_device_, &info, &command_buffer.vk_command_buffer_));
  command_buffer.vk_command_pool_ = vk_command_pool_;
  command_buffer.vk_device_ = vk_device_;
  return Result<CommandBuffer>(std::move(command_buffer));
}

Result<CommandBuffers> CommandPool::allocate(u32 count,
                                             VkCommandBufferLevel level) {
  VkCommandBufferAllocateInfo info;
  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  info.pNext = nullptr;
  info.commandPool = vk_command_pool_;
  info.level = level;
  info.commandBufferCount = count;
  std::vector<VkCommandBuffer> command_buffers(count);
  VENUS_VK_RETURN_BAD_RESULT(
      vkAllocateCommandBuffers(vk_device_, &info, command_buffers.data()));

  CommandBuffers cbs;
  for (auto &cb : command_buffers) {
    CommandBuffer command_buffer;
    command_buffer.vk_command_buffer_ = cb;
    command_buffer.vk_command_pool_ = vk_command_pool_;
    command_buffer.vk_device_ = vk_device_;
    cbs.emplace_back(std::move(command_buffer));
  }
  return Result<CommandBuffers>(std::move(cbs));
}

VeResult CommandPool::imadiateSubmit(
    u32 queue_family_index, VkQueue queue,
    const std::function<void(CommandBuffer &)> &record_callback) {
  CommandPool short_living_command_pool;
  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      short_living_command_pool,
      CommandPool::Config()
          .addCreateFlags(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT)
          .setQueueFamilyIndex(queue_family_index)
          .create(vk_device_));

  CommandBuffer short_living_command_buffer;
  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      short_living_command_buffer,
      short_living_command_pool.allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY));

  VENUS_RETURN_BAD_RESULT(short_living_command_buffer.begin(
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT));
  record_callback(short_living_command_buffer);
  VENUS_RETURN_BAD_RESULT(short_living_command_buffer.end());

  core::Fence submit_fence;
  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      submit_fence, core::Fence::Config().create(vk_device_));
  short_living_command_buffer.submit(queue, *submit_fence);
  VENUS_VK_RETURN_BAD_RESULT(submit_fence.wait());
  return VeResult::noError();
}

SubmitInfo2 &SubmitInfo2::addWaitInfo(VkPipelineStageFlags2 stage_mask,
                                      VkSemaphore semaphore) {
  wait_semaphores_.emplace_back(semaphoreSubmitInfo(stage_mask, semaphore));
  return *this;
}

SubmitInfo2 &SubmitInfo2::addSignalInfo(VkPipelineStageFlags2 stage_mask,
                                        VkSemaphore semaphore) {
  signal_semaphores_.emplace_back(semaphoreSubmitInfo(stage_mask, semaphore));
  return *this;
}

SubmitInfo2 &SubmitInfo2::addCommandBufferInfo(VkCommandBuffer cb) {
  VkCommandBufferSubmitInfo info{};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
  info.pNext = nullptr;
  info.commandBuffer = cb;
  info.deviceMask = 0;
  cb_infos_.emplace_back(info);
  return *this;
}

VkResult SubmitInfo2::submit(VkQueue queue, VkFence fence) const {
  VkSubmitInfo2 info = {};
  info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
  info.pNext = nullptr;

  info.waitSemaphoreInfoCount = wait_semaphores_.size();
  info.pWaitSemaphoreInfos = wait_semaphores_.data();

  info.signalSemaphoreInfoCount = signal_semaphores_.size();
  info.pSignalSemaphoreInfos = signal_semaphores_.data();

  info.commandBufferInfoCount = cb_infos_.size();
  info.pCommandBufferInfos = cb_infos_.data();

  return vkQueueSubmit2(queue, 1, &info, fence);
}

VkSemaphoreSubmitInfo
SubmitInfo2::semaphoreSubmitInfo(VkPipelineStageFlags2 stage_mask,
                                 VkSemaphore semaphore) {
  VkSemaphoreSubmitInfo info{};
  info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
  info.pNext = nullptr;
  info.semaphore = semaphore;
  info.stageMask = stage_mask;
  info.deviceIndex = 0;
  info.value = 1;
  return info;
}

BufferWritter &BufferWritter::addBuffer(VkBuffer buffer, const void *data,
                                        u32 size_in_bytes) {
  data_.emplace_back(data);
  sizes_.emplace_back(size_in_bytes);
  buffers_.emplace_back(buffer);
  return *this;
}

VeResult BufferWritter::record(const core::Device &device,
                               VkCommandBuffer cb) const {
  // compute total staging size
  std::vector<u32> offsets(1, 0);
  u32 staging_size = 0;
  for (const auto &size : sizes_) {
    offsets.emplace_back(staging_size + size);
    staging_size += size;
  }

  mem::AllocatedBuffer staging;
  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      staging,
      mem::AllocatedBuffer::Config()
          .setBufferConfig(mem::Buffer::Config::forStaging(staging_size))
          .setMemoryConfig(
              mem::DeviceMemory::Config()
                  .setAllocationFlags(VMA_ALLOCATION_CREATE_MAPPED_BIT)
                  .setUsage(VMA_MEMORY_USAGE_CPU_ONLY))
          .create(device));

  std::vector<VkBufferCopy> copies;
  for (u32 i = 0; i < data_.size(); ++i) {
    // transfer data to staging
    VENUS_RETURN_BAD_RESULT(staging.copy(data_[i], sizes_[i], offsets[i]));
  }

  // record staging -> device transfer
  for (u32 i = 0; i < data_.size(); ++i) {
    VkBufferCopy vertex_copy{0};
    vertex_copy.dstOffset = 0;
    vertex_copy.srcOffset = offsets[i];
    vertex_copy.size = sizes_[i];
    vkCmdCopyBuffer(cb, *staging, buffers_[i], 1, &vertex_copy);
  }

  return VeResult::noError();
}

VeResult
BufferWritter::immediateSubmit(const engine::GraphicsDevice &gd) const {
  // compute total staging size
  std::vector<u32> offsets(1, 0);
  u32 staging_size = 0;
  for (const auto &size : sizes_) {
    offsets.emplace_back(staging_size + size);
    staging_size += size;
  }

  mem::AllocatedBuffer staging;
  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      staging,
      mem::AllocatedBuffer::Config()
          .setBufferConfig(mem::Buffer::Config::forStaging(staging_size))
          .setMemoryConfig(
              mem::DeviceMemory::Config()
                  .setAllocationFlags(VMA_ALLOCATION_CREATE_MAPPED_BIT)
                  .setUsage(VMA_MEMORY_USAGE_CPU_ONLY))
          .create(*gd));

  std::vector<VkBufferCopy> copies;
  for (u32 i = 0; i < data_.size(); ++i) {
    // transfer data to staging
    VENUS_RETURN_BAD_RESULT(staging.copy(data_[i], sizes_[i], offsets[i]));
  }

  VENUS_RETURN_BAD_RESULT(
      gd.immediateSubmit([&](const pipeline::CommandBuffer &cb) {
        // record staging -> device transfer
        for (u32 i = 0; i < data_.size(); ++i) {
          VkBufferCopy vertex_copy{0};
          vertex_copy.dstOffset = 0;
          vertex_copy.srcOffset = offsets[i];
          vertex_copy.size = sizes_[i];
          vkCmdCopyBuffer(*cb, *staging, buffers_[i], 1, &vertex_copy);
        }
      }));

  return VeResult::noError();
}

} // namespace venus::pipeline
