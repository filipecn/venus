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

/// \file   buffer.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30
/// \brief  Vulkan buffer

#pragma once

#include <venus/mem/device_memory.h>

#include <optional>

namespace venus::mem {

/// \brief Holds a vulkan buffer object.
/// A buffer is a region of memory that can be used to store data that can be
/// accessed by the CPU and the GPU. The buffer is created on the device memory,
/// and it can be mapped to the CPU memory.
///
/// Buffers represent linear arrays of data, just like C arrays, and is one of
/// the types of resources provided by Vulkan. Buffers can be used as storage
/// buffers, textel buffers, used as a source of data for vertex attributes, and
/// various other purposes.
/// \note this class uses raii.
class Buffer {
public:
  /// Builder for buffer.
  /// \note The builder considers device local buffers by default and that data
  ///       will be transferred through staging buffers.
  struct Config {
    friend class Buffer;
    /// \brief Config for staging buffers
    /// \note This sets usage flag as eTransferSrc
    /// \param size_in_bytes
    static Config forStaging(const VkDeviceSize &size_in_bytes);
    /// \brief Config for uniform buffers
    /// \note This sets usage flag as eUniformBuffer.
    /// \note This sets for device local storage.
    /// \param size_in_bytes
    static Config forUniform(const VkDeviceSize &size_in_bytes);
    /// \brief Config for storage buffers.
    /// \note This sets usage flag as eStorageBuffer
    /// \note This sets usage flag as eTransferDst
    /// \note This sets for device local storage.
    /// \param size_in_bytes
    static Config forStorage(const VkDeviceSize &size_in_bytes);
    /// \brief Config for index buffers.
    /// \note This sets usage flag as eIndexBuffer
    /// \note This sets usage flag as eTransferDst
    /// \note This sets for device local storage.
    /// \param index_count
    /// \param index_type [def=VK_INDEX_TYPE_UINT32] Index data type.
    static Config forIndices(u32 index_count,
                             VkIndexType index_type = VK_INDEX_TYPE_UINT32);
    /// Set buffer size.
    /// \param size_in_bytes
    Config &setSize(VkDeviceSize size_in_bytes);
    /// Append buffer usage.
    ///  - VK_BUFFER_USAGE_TRANSFER_SRC_BIT specifies that the buffer can be a
    ///    source of data for copy operations
    ///  - VK_BUFFER_USAGE_TRANSFER_DST_BIT specifies that we can copy data to
    ///    the buffer.
    ///  - VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT indicates that the buffer
    ///    can be used in shaders as a uniform texel buffer.
    ///  - VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT specifies that the buffer
    ///    can be used in shaders as a storage texel buffer.
    ///  - VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT indicates that the buffer can be
    ///    used in shaders as a source of values for uniform variables
    ///  - VK_BUFFER_USAGE_STORAGE_BUFFER_BIT indicates that we can store data
    ///    in the buffer from within shaders.
    ///  - VK_BUFFER_USAGE_INDEX_BUFFER_BIT specifies that the buffer can be
    ///    used as a source of vertex indices during drawing.
    ///  - VK_BUFFER_USAGE_VERTEX_BUFFER_BIT indicates that the buffer can be a
    ///    source of data for vertex attributes specified during drawing.
    ///  - VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT indicates that the buffer can
    ///    contain data that will be used during indirect drawing.
    /// \param usage
    Config &addUsage(VkBufferUsageFlags usage);
    /// \param mode Sharing mode.
    Config &setSharingMode(VkSharingMode mode);
    /// \note This sets memory usage enableShaderDeviceAddress
    Config &enableShaderDeviceAddress();
    /// \param flags
    Config &addCreateFlags(VkBufferCreateFlags flags);
    ///
    HERMES_NODISCARD VkBufferCreateInfo createInfo() const;
    /// \brief Creates a buffer object from this configuration.
    /// \param device
    /// \return Buffer object or error.
    HERMES_NODISCARD Result<Buffer> create(const core::Device &device) const;

  private:
    VkDeviceSize size_{};                                   //< size in bytes
    VkBufferUsageFlags usage_{};                            //< buffer purpose
    VkSharingMode sharing_mode_{VK_SHARING_MODE_EXCLUSIVE}; //< sharing mode
    VkBufferCreateFlags flags_{};

    VENUS_TO_STRING_FRIEND(Buffer::Config);
  };
  /// Buffer views allow us to define how buffer's memory is accessed and
  /// interpreted. For example, we can choose to look at the buffer as a uniform
  /// texel buffer or as a storage texel buffer.
  /// \note this uses raii.
  class View final {
    friend class Buffer;

  public:
    /// Builder for Buffer::View
    struct Config {
      Config &setFormat(VkFormat format);
      Config &setRange(VkDeviceSize range);
      Config &setOffset(VkDeviceSize offset);

      Result<View> create(const Buffer &buffer) const;

    private:
      VkFormat format_{};
      VkDeviceSize offset_{0};
      VkDeviceSize range_{0};
    };

    // raii

    VENUS_DECLARE_RAII_FUNCTIONS(View);

    void swap(View &rhs) noexcept;
    void destroy() noexcept;
    VkBufferView operator*() const;

  private:
    VkDevice vk_device_{VK_NULL_HANDLE};
    VkBufferView vk_buffer_view_{VK_NULL_HANDLE};
  };

  // raii

  VENUS_DECLARE_RAII_FUNCTIONS(Buffer);

  /// Information about the type of memory and how much of it the
  /// buffer resource requires.
  HERMES_NODISCARD VkMemoryRequirements memoryRequirements() const;
  /// \return Buffer size in bytes.
  VkDeviceSize sizeInBytes() const;
  /// \return Buffer device address.
  HERMES_NODISCARD VkDeviceAddress deviceAddress() const;

  /// Frees memory and destroy buffer/memory objects.
  virtual void destroy() noexcept;
  //
  void swap(Buffer &rhs) noexcept;
  /// \return Underlying vk buffer object.
  VkBuffer operator*() const;
  /// \return Associated logical device.
  VkDevice device() const;

protected:
  void init(VkDevice vk_device, VkBuffer vk_buffer);

  VkMemoryRequirements vk_memory_requirements_{};
  VkBuffer vk_buffer_{VK_NULL_HANDLE};
  VkDevice vk_device_{VK_NULL_HANDLE};
  std::optional<VkDeviceAddress> vk_device_address_;

private:
#ifdef VENUS_DEBUG
  Config config_{};
#endif

  VENUS_TO_STRING_FRIEND(Buffer);
};

/// \brief Holds a self-allocated vulkan buffer object.
/// The AllocatedBuffer owns the device memory used by the buffer.
class AllocatedBuffer : public Buffer, public DeviceMemory {
public:
  struct Config {
    Config &setBufferConfig(const Buffer::Config &config);
    Config &setMemoryConfig(const DeviceMemory::Config &config);

    Result<AllocatedBuffer> create(const core::Device &device) const;

  private:
    Buffer::Config buffer_config_;
    DeviceMemory::Config mem_config_;

    VENUS_TO_STRING_FRIEND(Config);
  };

  VENUS_DECLARE_RAII_FUNCTIONS(AllocatedBuffer)

  /// Frees memory and destroy buffer/memory objects.
  void destroy() noexcept override;
  //
  void swap(AllocatedBuffer &rhs) noexcept;

private:
  VENUS_TO_STRING_FRIEND(AllocatedBuffer);
};

} // namespace venus::mem
