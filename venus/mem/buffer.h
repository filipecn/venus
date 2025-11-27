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
  /// Builder for Buffer.
  /// \tparam Derived return type of configuration methods.
  /// \tparam Type type of the object build by this setup.
  /// \note The builder considers device local buffers by default and that data
  ///       will be transferred through staging buffers.
  template <typename Derived> struct Setup {
    friend class Buffer;
    /// \brief Config for staging buffers
    /// \note This sets usage flag as eTransferSrc
    /// \param size_in_bytes
    static Derived forStaging(const VkDeviceSize &size_in_bytes);
    /// \brief Setup for uniform buffers
    /// \note This sets usage flag as eUniformBuffer.
    /// \note This sets for device local storage.
    /// \param size_in_bytes
    static Derived forUniform(const VkDeviceSize &size_in_bytes);
    /// \brief Setup for storage buffers.
    /// \note This sets usage flag as eStorageBuffer
    /// \note This sets usage flag as eTransferDst
    /// \note This sets for device local storage.
    /// \param size_in_bytes
    static Derived forStorage(const VkDeviceSize &size_in_bytes);
    /// \brief Setup for index buffers.
    /// \note This sets usage flag as eIndexBuffer
    /// \note This sets usage flag as eTransferDst
    /// \note This sets for device local storage.
    /// \param index_count
    /// \param index_type [def=VK_INDEX_TYPE_UINT32] Index data type.
    static Derived forIndices(u32 index_count,
                              VkIndexType index_type = VK_INDEX_TYPE_UINT32);
    Setup() noexcept;
    /// Set buffer size.
    /// \param size_in_bytes
    Derived &setSize(VkDeviceSize size_in_bytes);
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
    ///  - VK_BUFFER_USAGE_VERTEX_BUFFER_BIT indicates that the buffer can be
    ///  a
    ///    source of data for vertex attributes specified during drawing.
    ///  - VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT indicates that the buffer can
    ///    contain data that will be used during indirect drawing.
    /// \param usage
    Derived &addUsage(VkBufferUsageFlags usage);
    /// \param mode Sharing mode.
    Derived &setSharingMode(VkSharingMode mode);
    /// \note This sets memory usage enableShaderDeviceAddress
    Derived &enableShaderDeviceAddress();
    /// \param flags
    Derived &addCreateFlags(VkBufferCreateFlags flags);
    ///
    HERMES_NODISCARD VkBufferCreateInfo createInfo() const;

  protected:
    VkDeviceSize size_{};                                   //< size in bytes
    VkBufferUsageFlags usage_{};                            //< buffer purpose
    VkSharingMode sharing_mode_{VK_SHARING_MODE_EXCLUSIVE}; //< sharing mode
    VkBufferCreateFlags flags_{};
  };
  struct Config : public Setup<Config> {
    /// \brief Creates a buffer object from this configuration.
    /// \param device
    /// \return Buffer object or error.
    HERMES_NODISCARD Result<Buffer> build(const core::Device &device) const;

    VENUS_to_string_FRIEND(Buffer::Config);
  };
  /// Buffer views allow us to define how buffer's memory is accessed and
  /// interpreted. For example, we can choose to look at the buffer as a
  /// uniform texel buffer or as a storage texel buffer.
  /// \note this uses raii.
  class View final {
    friend class Buffer;

  public:
    /// Builder for Buffer::View
    struct Config {
      Config &setFormat(VkFormat format);
      Config &setRange(VkDeviceSize range);
      Config &setOffset(VkDeviceSize offset);

      Result<View> build(const Buffer &buffer) const;

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

  VENUS_to_string_FRIEND(Buffer);
};

template <typename Derived> Buffer::Setup<Derived>::Setup() noexcept {}

template <typename Derived>
Derived Buffer::Setup<Derived>::forStaging(const VkDeviceSize &size_in_bytes) {
  return Buffer::Setup<Derived>()
      .addUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
      .setSize(size_in_bytes);
}

template <typename Derived>
Derived Buffer::Setup<Derived>::forUniform(const VkDeviceSize &size_in_bytes) {
  return Buffer::Setup<Derived>()
      .addUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
      .addUsage(VK_BUFFER_USAGE_TRANSFER_DST_BIT)
      .setSize(size_in_bytes);
}

template <typename Derived>
Derived Buffer::Setup<Derived>::forStorage(const VkDeviceSize &size_in_bytes) {
  return Buffer::Setup<Derived>()
      .addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
      .addUsage(VK_BUFFER_USAGE_TRANSFER_DST_BIT)
      .setSize(size_in_bytes);
}

template <typename Derived>
Derived Buffer::Setup<Derived>::forIndices(u32 index_count,
                                           VkIndexType index_type) {
  return Buffer::Setup<Derived>()
      .addUsage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
      .addUsage(VK_BUFFER_USAGE_TRANSFER_DST_BIT)
      .setSize(index_count * core::vk::indexSize(index_type));
}

VENUS_DEFINE_SETUP_SET_FIELD_METHOD(Buffer, setSize, VkDeviceSize, size_)

VENUS_DEFINE_SETUP_ADD_FLAGS_METHOD(Buffer, addUsage, VkBufferUsageFlags,
                                    usage_)

VENUS_DEFINE_SETUP_SET_FIELD_METHOD(Buffer, setSharingMode, VkSharingMode,
                                    sharing_mode_)

VENUS_DEFINE_SETUP_ADD_FLAGS_METHOD(Buffer, addCreateFlags, VkBufferCreateFlags,
                                    flags_)

template <typename Derived>
Derived &Buffer::Setup<Derived>::enableShaderDeviceAddress() {
  usage_ |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
  return static_cast<Derived &>(*this);
}

template <typename Derived>
VkBufferCreateInfo Buffer::Setup<Derived>::createInfo() const {
  VkBufferCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  info.pNext = nullptr;
  info.flags = flags_;
  info.size = size_;
  info.usage = usage_;
  info.sharingMode = sharing_mode_;
  info.queueFamilyIndexCount = 0;
  info.pQueueFamilyIndices = nullptr;
  return info;
}

/// \brief Holds a self-allocated vulkan buffer object.
/// The AllocatedBuffer owns the device memory used by the buffer.
class AllocatedBuffer : public Buffer, public DeviceMemory {
public:
  struct Config : public Buffer::Setup<Config>,
                  public DeviceMemory::Setup<Config> {
    static Config forStaging(u32 size_in_bytes);
    static Config forUniform(u32 size_in_bytes);
    static Config forStorage(u32 size_in_bytes, VkBufferUsageFlags usage);
    static Config forAccelerationStructure(u32 size_in_bytes);
    static Config forShaderBindingTable(u32 size_in_bytes);

    Result<AllocatedBuffer> build(const core::Device &device) const;

  private:
    Buffer::Config buffer_config_;
    DeviceMemory::Config mem_config_;

    VENUS_to_string_FRIEND(Config);
  };

  VENUS_DECLARE_RAII_FUNCTIONS(AllocatedBuffer)

  /// Frees memory and destroy buffer/memory objects.
  void destroy() noexcept override;
  //
  void swap(AllocatedBuffer &rhs) noexcept;
  operator bool() const;

private:
  VENUS_to_string_FRIEND(AllocatedBuffer);
};

/// Allocated buffer pools hold multiple allocated buffers that can be labeled
/// and accessed.
/// They can be very handy for uniform buffers that can be share by multiple
/// descriptor sets.
class BufferPool {
public:
  VENUS_DECLARE_RAII_FUNCTIONS(BufferPool)

  void destroy() noexcept;
  void swap(BufferPool &rhs);

  /// Allocates a new buffer with label name.
  /// \param name Buffer label.
  /// \param config Allocated buffer config.
  /// \return error.
  template <class... P>
  VeResult addBuffer(const std::string &name,
                     const AllocatedBuffer::Config &config, P &&...params) {
    BufferData data{};
    VENUS_ASSIGN_OR_RETURN_BAD_RESULT(data.buffer,
                                      config.build(std::forward<P>(params)...));
    buffers_[name] = std::move(data);
    return VeResult::noError();
  }
  /// Copies data into buffer.
  /// \param name Buffer key (label).
  /// \param block_index index inside buffer.
  /// \param data Data being copied.
  /// \param size_in_bytes Size of the data being copied.
  /// \param offset_in_block Offset in bytes (within block) of the copy.
  /// \return error.
  HERMES_NODISCARD VeResult copyBlock(const std::string &name, u32 block_index,
                                      const void *data, u32 size_in_bytes,
                                      u32 offset_in_block = 0);
  /// Destroys buffer.
  /// \param name Buffer key (label).
  void removeBuffer(const std::string &name);
  /// Gets buffer vulkan object.
  /// \param name Buffer key (label).
  /// \return Buffer or error.
  HERMES_NODISCARD Result<VkBuffer> operator[](const std::string &name) const;
  /// Appends count blocks in a buffer free space.
  /// \param name Buffer key (label).
  /// \param size_in_bytes [def=0] Size of the block to be allocated. If 0,
  ///                              allocates whole available size.
  /// \param count [def=1] Number of same-size blocks to be allocated
  ///                      contiguously in the buffer.
  /// \return The offset in the buffer of the first allocated block.
  HERMES_NODISCARD Result<u32> allocate(const std::string &name,
                                        u32 size_in_bytes = 0, u32 count = 1);
  /// Appends count blocks of sizeof(T) in a buffer free space.
  /// \param name Buffer key (label).
  /// \param count [def=1] Number of same-size blocks to be allocated
  ///                      contiguously in the buffer.
  /// \return The offset in the buffer of the first allocated block.
  template <typename T>
  HERMES_NODISCARD Result<u32> allocate(const std::string &name,
                                        u32 count = 1) {
    return allocate(name, sizeof(T), count);
  }
  /// Retrieves the offset of a given allocated block index.
  /// \param name Buffer key (label).
  /// \param block_index Block indices are 0-based sequential numbers.
  HERMES_NODISCARD Result<u32> blockOffset(const std::string &name,
                                           u32 block_index) const;

private:
  VENUS_to_string_FRIEND(BufferPool);

  struct BufferData {
    AllocatedBuffer buffer;
    std::vector<u32> block_offsets;
    u32 size{0};
  };

  std::unordered_map<std::string, BufferData> buffers_;
};

} // namespace venus::mem
