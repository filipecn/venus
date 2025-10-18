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

/// \file   descriptors.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30
/// \brief  Classes for manipulating descriptor sets.

#pragma once

#include <venus/mem/buffer.h>
#include <venus/pipeline/pipeline.h>

namespace venus::engine {
class GraphicsDevice;
}

namespace venus::pipeline {

/// Command buffers record operations and are submitted to the hardware. They
/// can be recorded in multiple threads and also can be saved and reused.
/// Synchronization is very important on this part, because the operations
/// submitted need to be processed properly.
/// Before allocating command buffers, we need to allocate command pools,
/// from which the command buffers acquire memory. Command pools are also
/// responsible for informing the driver on how to deal with the command
/// buffers memory allocated from them (for example, whether the command
/// buffer will have a short life or if they need to be reset or freed).
/// Command pools also control the queues that receive the command buffers.
/// Command buffers are separate in two groups (levels):
/// 1. Primary - can be directly submitted to queues and call secondary
/// command
///    buffers.
/// 2. Secondary - can only be executed from primary command buffers.
/// When recording commands to command buffers, we need to set the state for
/// the operations as well (for example, vertex attributes use other buffers
/// to work). When calling a secondary command buffer, the state of the
/// caller primary command buffer is not preserved (unless it is a render
/// pass).
/// \note Command buffers can only be created by command pools.
/// \note This uses RAII.
class CommandBuffer {
public:
  struct RenderingInfo {
    struct Attachment {
      Attachment() noexcept;
      Attachment &setImageView(VkImageView image_view);
      Attachment &setImageLayout(VkImageLayout image_layout);
      Attachment &setResolveMode(VkResolveModeFlagBits resolve_mode);
      Attachment &setResolveImageView(VkImageView resolve_image_view);
      Attachment &setResolveImageLayout(VkImageLayout resolve_image_layout);
      Attachment &setLoadOp(VkAttachmentLoadOp load_op);
      Attachment &setStoreOp(VkAttachmentStoreOp store_op);
      Attachment &setClearValue(VkClearValue clear_value);
      VkRenderingAttachmentInfo operator*() const;

    private:
      VkRenderingAttachmentInfo info_{};
    };

    RenderingInfo() noexcept;
    RenderingInfo &setFlags(VkRenderingFlags flags);
    RenderingInfo &setRenderArea(const VkRect2D &render_area);
    RenderingInfo &setLayerCount(u32 layer_count);
    RenderingInfo &setViewMask(u32 view_mask);
    RenderingInfo &addColorAttachment(const Attachment &color_attachment);
    RenderingInfo &setDepthAttachment(const Attachment &depth_attachment);
    RenderingInfo &setStencilAttachment(const Attachment &stencil_attachment);
    VkRenderingInfo operator*() const;

  private:
    VkRenderingInfo info_{};
    std::vector<VkRenderingAttachmentInfo> color_attachments_;
    std::optional<VkRenderingAttachmentInfo> depth_attachment_;
    std::optional<VkRenderingAttachmentInfo> stencil_attachment_;
  };

  struct RenderPassInfo {
    RenderPassInfo() noexcept;
    RenderPassInfo &setRenderArea(i32 x, i32 y, u32 width, u32 height);
    RenderPassInfo &addClearColorValuef(f32 r, f32 g, f32 b, f32 a);
    RenderPassInfo &addClearColorValuei(i32 r, i32 g, i32 b, i32 a);
    RenderPassInfo &addClearColorValueu(u32 r, u32 g, u32 b, u32 a);
    RenderPassInfo &addClearDepthStencilValue(f32 depth, u32 stencil);

    VkRenderPassBeginInfo info(VkRenderPass vk_renderpass,
                               VkFramebuffer vk_framebuffer) const;

  private:
    VkRenderPassBeginInfo info_{};
    std::vector<VkClearValue> clear_values_;
  };

  VENUS_DECLARE_RAII_FUNCTIONS(CommandBuffer)

  void destroy() noexcept;
  void swap(CommandBuffer &rhs) noexcept;
  VkCommandBuffer operator*() const;

  HERMES_NODISCARD VeResult begin(VkCommandBufferUsageFlags flags = 0) const;
  HERMES_NODISCARD VeResult end() const;
  ///\param flags
  HERMES_NODISCARD VeResult reset(VkCommandBufferResetFlags flags = 0) const;
  VeResult submit(VkQueue queue, VkFence fence = VK_NULL_HANDLE) const;
  /// \param vk_src_buffer
  /// \param vk_dst_buffer
  /// \param vk_copy_region
  void copy(VkBuffer vk_src_buffer, VkBuffer vk_dst_buffer,
            const std::vector<VkBufferCopy> &regions) const;
  /// \param vk_src_buffer
  /// \param vk_dst_buffer
  /// \param vk_copy_region
  void copy(VkBuffer vk_src_buffer, VkBuffer vk_dst_buffer,
            VkBufferCopy vk_copy_region) const;
  /// \param src_buffer
  /// \param dst_image
  /// \param layout Layout that the image is expected to be in when the copy
  ///               command is executed. Accepted layouts are
  ///               VK_IMAGE_LAYOUT_[GENERAL | TRANSFER_DST_OPTIMAL]. To clear
  ///               images in different layouts you need to move them to one of
  ///               these two layouts using a pipeline barrier before the clear
  ///               command.
  /// \param regions
  void copy(VkBuffer src_buffer, VkImage dst_image, VkImageLayout layout,
            const std::vector<VkBufferImageCopy> &regions) const;
  ///\param src_image
  ///\param dst_buffer
  ///\param layout Layout that the image is expected to be in when the
  /// copy command is executed. Accepted layouts are VK_IMAGE_LAYOUT_[GENERAL |
  /// TRANSFER_DST_OPTIMAL]. To clear images in different layouts you need to
  /// move them to one of these two layouts using a pipeline barrier before the
  /// clear command.
  ///\param regions
  void copy(VkImage src_image, VkImageLayout layout, VkBuffer dst_buffer,
            const std::vector<VkBufferImageCopy> &regions) const;
  /// \param src_image
  /// \param src_layout
  /// \param dst_image
  /// \param dst_layout
  /// \param regions
  void copy(VkImage src_image, VkImageLayout src_layout, VkImage dst_image,
            VkImageLayout dst_layout,
            const std::vector<VkImageCopy> &regions) const;
  /// Fills a buffer with a fixed value.
  ///\tparam T
  ///\param buffer
  ///\param data
  ///\param offset  multiple of 4
  ///\param length
  template <typename T>
  void fill(const mem::Buffer &buffer, T data, VkDeviceSize offset = 0,
            VkDeviceSize length = VK_WHOLE_SIZE) const {
    vkCmdFillBuffer(vk_command_buffer_, *buffer, offset, length,
                    *(const u32 *)&data);
  }
  /// Copies data directly from host memory into a buffer oject.
  ///\tparam T
  ///\param buffer
  ///\param data
  ///\param offset  multiple of 4
  ///\param length
  template <typename T>
  void update(const mem::Buffer &buffer, const T *data, VkDeviceSize offset,
              VkDeviceSize length) const {
    vkCmdUpdateBuffer(vk_command_buffer_, *buffer, offset, length,
                      (const u32 *)data);
  }
  /// Clears an image to a fixed color value
  /// \param image
  /// \param layout Layout that the image is expected to be in when the clear
  ///               command is executed. Accepted layouts are
  ///               VK_IMAGE_LAYOUT_[GENERAL | TRANSFER_DST_OPTIMAL]. To clear
  ///               images in different layouts you need to move them to one of
  ///               these two layouts using a pipeline barrier before the clear
  ///               command.
  /// \param ranges regions to be cleared. Note: aspectMask must be set to
  ///               VK_IMAGE_ASPECT_COLOR_BIT.
  /// \param color  Note: it is up to the application to fill the color union
  ///               object correctly, no conversion is performed by the clear
  ///               command.
  void clear(VkImage image, VkImageLayout layout,
             const std::vector<VkImageSubresourceRange> &ranges,
             const VkClearColorValue &color) const;
  /// Clears a depth stencil image to a fixed value
  ///\param image
  ///\param layout  layout that the image is expected to be in when the
  /// clear command is executed. Accepted layouts are VK_IMAGE_LAYOUT_[GENERAL |
  /// TRANSFER_DST_OPTIMAL]. To clear images in different layouts you need to
  /// move them to one of these two layouts using a pipeline barrier before the
  /// clear command.
  ///\param ranges  regions to be cleared. Note: aspectMask must be set
  /// to VK_IMAGE_ASPECT_[DEPTH_BIT or/and STENCIL_BIT].
  ///\param value
  ///\return bool true if success
  void clear(VkImage image, VkImageLayout layout,
             const std::vector<VkImageSubresourceRange> &ranges,
             const VkClearDepthStencilValue &value) const;
  /// \param compute_pipeline
  void bind(const ComputePipeline &compute_pipeline) const;
  /// \param graphics_pipeline
  void bind(const GraphicsPipeline &graphics_pipeline) const;
  /// \param pipeline_bind_point
  /// \param layout
  /// \param first_set
  /// \param descriptor_sets
  /// \param dynamic_offsets
  /// \return
  void bind(VkPipelineBindPoint pipeline_bind_point, VkPipelineLayout layout,
            u32 first_set, const std::vector<VkDescriptorSet> &descriptor_sets,
            const std::vector<u32> &dynamic_offsets = {}) const;
  /// \param pipeline_bind_point  VK_PIPELINE_BIND_POINT_[COMPUTE | GRAPHICS]
  /// \param pipeline_layout      the layout that will be used by pipelines that
  ///                             will access the descriptors
  /// \param descriptor_sets
  /// \param dynamic_offsets      offsets used in dynamic uniform or shader
  ///                             storage bindings
  /// \param first_set            index of the first set to bind
  /// \param descriptor_set_count
  void bind(VkPipelineBindPoint pipeline_bind_point,
            VkPipelineLayout pipeline_layout,
            const std::vector<VkDescriptorSet> &descriptor_sets,
            const std::vector<u32> &dynamic_offsets, u32 first_set,
            u32 descriptor_set_count) const;
  /// Dispatches a glocal work group
  /// \note A valid ComputePipeline must be bound to the command buffer
  /// \param x  number of local work groups in x
  /// \param y  number of local work groups in y
  /// \param z  number of local work groups in z
  void dispatch(u32 x, u32 y, u32 z) const;
  /// \brief Performs an indirect dispatch
  ///  The size of the dispatch in work groups is sourced from a buffer object.
  /// \param buffer  buffer storing the x, y and z values contiguously
  /// \param offset  location of the workgroup sizes in the buffer (in bytes)
  void dispatch(VkBuffer buffer, VkDeviceSize offset) const;
  /// The content of each push constant lives at an offset from the beginning of
  /// the block.
  /// \param pipeline_layout
  /// \param stage_flags  that stages that will need to see the updated
  ///  constants
  /// \param offset
  /// \param size  (in bytes)
  /// \param values
  /// \return bool
  void pushConstants(VkPipelineLayout pipeline_layout,
                     VkShaderStageFlags stage_flags, u32 offset, u32 size,
                     const void *values) const;
  /// Sets the current renderpass object and configures the set of output images
  /// that will be drawn into.
  /// \param info  Parameters describing the renderpass
  /// \param contents
  void beginRenderPass(const VkRenderPassBeginInfo &info,
                       VkSubpassContents contents) const;
  /// Finalize rendering contained in the renderpass
  void endRenderPass() const;
  /// Initializes dynamic rendering
  void beginRendering(const VkRenderingInfo &info) const;
  /// Finalize dynamic rendering
  void endRendering() const;
  /// \param first_binding
  /// \param buffers
  /// \param offsets
  void bindVertexBuffers(u32 first_binding,
                         const std::vector<VkBuffer> &buffers,
                         const std::vector<VkDeviceSize> &offsets) const;
  void bindIndexBuffer(VkBuffer buffer, VkDeviceSize offset,
                       VkIndexType type) const;
  /// Generates vertices and push them into the current graphics pipeline.
  /// \param vertex_count
  /// \param instance_count
  /// \param first_vertex
  /// \param first_instance
  [[maybe_unused]] void draw(u32 vertex_count, u32 instance_count = 1,
                             u32 first_vertex = 0,
                             u32 first_instance = 0) const;
  void drawIndexed(u32 index_count, u32 instance_count = 1, u32 first_index = 0,
                   i32 vertex_offset = 0, u32 first_instance = 0) const;
  /// \param barrier
  /// \param src_stages
  /// \param dst_stages
  void transitionImageLayout(VkImageMemoryBarrier barrier,
                             VkPipelineStageFlags src_stages,
                             VkPipelineStageFlags dst_stages) const;
  /// \param src_image
  /// \param src_image_layout
  /// \param dst_image
  /// \param dst_image_layout
  /// \param regions
  /// \param filter
  void blit(VkImage src_image, VkImageLayout src_image_layout,
            VkImage dst_image, VkImageLayout dst_image_layout,
            const std::vector<VkImageBlit> &regions, VkFilter filter) const;
  /// Set the Viewport object
  /// \param width
  /// \param height
  /// \param min_depth
  /// \param max_depth
  void setViewport(f32 width, f32 height, f32 min_depth, f32 max_depth) const;
  /// Set the Scissor object
  /// \param offset_x
  /// \param offset_y
  /// \param extent_width
  /// \param extent_height
  void setScissor(i32 offset_x, i32 offset_y, u32 extent_width,
                  u32 extent_height) const;
  ///
  void transitionImage(VkImage vk_image, VkImageLayout current_layout,
                       VkImageLayout new_layout) const;

private:
  friend class CommandPool;

  VkCommandBuffer vk_command_buffer_{VK_NULL_HANDLE};
  VkCommandPool vk_command_pool_{VK_NULL_HANDLE};
  VkDevice vk_device_{VK_NULL_HANDLE};
};

class CommandBuffers : public std::vector<CommandBuffer> {};

/// \note Command pools cannot be used concurrently, we must create a separate
///       command pool for each thread.
class CommandPool {
public:
  struct Config {
    Config &addCreateFlags(VkCommandPoolCreateFlagBits flags);
    Config &setQueueFamilyIndex(u32 queue_family_index);

    Result<CommandPool> create(VkDevice vk_device) const;

  private:
    VkCommandPoolCreateFlags flags_{};
    u32 queue_family_index_{0};
  };

  VENUS_DECLARE_RAII_FUNCTIONS(CommandPool)

  Result<CommandBuffer>
  allocate(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
  Result<CommandBuffers>
  allocate(u32 count,
           VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
  HERMES_NODISCARD VeResult reset(VkCommandPoolResetFlags flags) const;
  VeResult
  imadiateSubmit(u32 queue_family_index, VkQueue queue,
                 const std::function<void(CommandBuffer &)> &record_callback);

  void destroy() noexcept;
  void swap(CommandPool &rhs) noexcept;
  VkCommandPool operator*() const;

private:
  VkCommandPool vk_command_pool_{VK_NULL_HANDLE};
  VkDevice vk_device_{VK_NULL_HANDLE};
};

struct SubmitInfo2 {
  SubmitInfo2 &addWaitInfo(VkPipelineStageFlags2 stage_mask,
                           VkSemaphore semaphore);
  SubmitInfo2 &addSignalInfo(VkPipelineStageFlags2 stage_mask,
                             VkSemaphore semaphore);
  SubmitInfo2 &addCommandBufferInfo(VkCommandBuffer cb);
  VkResult submit(VkQueue queue, VkFence fence) const;

private:
  VkSemaphoreSubmitInfo semaphoreSubmitInfo(VkPipelineStageFlags2 stage_mask,
                                            VkSemaphore semaphore);

  std::vector<VkSemaphoreSubmitInfo> wait_semaphores_;
  std::vector<VkSemaphoreSubmitInfo> signal_semaphores_;
  std::vector<VkCommandBufferSubmitInfo> cb_infos_;
};

/// \brief Helper class to copy data into buffers from a single source.
/// The BufferWritter utilizes a staging buffer that concentrates the
/// data that is distributed into different destination buffers.
struct BufferWritter {
  BufferWritter &addBuffer(VkBuffer buffer, const void *data,
                           u32 size_in_bytes);
  VeResult record(const core::Device &device, VkCommandBuffer cb) const;
  VeResult immediateSubmit(const engine::GraphicsDevice &gd) const;

private:
  std::vector<const void *> data_;
  std::vector<u32> sizes_;
  std::vector<VkBuffer> buffers_;
};

/// \brief Helper class to copy data into images from a single source.
/// The ImageWritter utilizes a staging buffer that concentrates the
/// data that is distributed into different destination images.
struct ImageWritter {
  ImageWritter &addImage(VkImage image, const void *data,
                         const VkExtent2D &size);
  ImageWritter &addImage(VkImage image, const void *data,
                         const VkExtent3D &size);
  VeResult immediateSubmit(const engine::GraphicsDevice &gd) const;

private:
  VeResult generateMipmaps(const pipeline::CommandBuffer &cb, VkImage image,
                           VkExtent2D size) const;

  std::vector<const void *> data_;
  std::vector<VkExtent3D> sizes_;
  std::vector<VkImage> images_;
};

} // namespace venus::pipeline
