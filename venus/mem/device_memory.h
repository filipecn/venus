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

/// \file   device_memory.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30
/// \brief  Vulkan device memory

#pragma once

#include <venus/core/device.h>

namespace venus::mem {

/// Holds memory allocated in the device memory.
/// \note This class uses RAII.
class DeviceMemory {
public:
  /// Builder for device memory.
  struct Config {
    /// \param flags allocation flags (vma)
    Config &setAllocationFlags(VmaAllocationCreateFlags flags);
    /// \param usage memory usage (vma)
    Config &setUsage(VmaMemoryUsage usage);
    /// \param properties appending properties.
    Config &addRequiredProperties(VkMemoryPropertyFlags properties);
    /// \param properties appending properties.
    Config &addPreferredProperties(VkMemoryPropertyFlags properties);
    /// \param Bitmask containing one bit set for every memory type acceptable
    ///        for this allocation.
    Config &addMemoryType(u32 type_bits);
    /// \param pool (vma)
    Config &setPool(VmaPool pool);
    /// \param priority value in [0,1]
    Config &setPriority(f32 priority);
    /// \param requirements new memory requirements.
    Config &setRequirements(const VkMemoryRequirements &requirements);
    /// Set memory host visible.
    /// \note This sets memory properties eHostCoherent and eHostVisible.
    Config &setHostVisible();
    /// Set memory host visible.
    /// \note This sets memory properties eDeviceLocal.
    Config &setDeviceLocal();
    ///
    VmaAllocationCreateInfo allocationInfo() const;
    /// Creates a new DeviceMemory from this configuration.
    /// \param device Device holding the new allocated memory.
    /// \return a new DeviceMemory, or error.
    HERMES_NODISCARD Result<DeviceMemory> create(const core::Device &device);

  private:
    VkMemoryRequirements requirements_{};
    VmaAllocationCreateInfo vma_allocation_{};

    VENUS_TO_STRING_FRIEND(DeviceMemory::Config);
  };

  // raii

  VENUS_DECLARE_RAII_FUNCTIONS(DeviceMemory)

  /// \note Sometimes the driver may not immediately copy the data into the
  ///       buffer memory. It is also possible that writes to the buffer are not
  ///       visible in the mapped memory yet. There are two ways to deal with
  ///       that problem:
  ///         - Use a memory heap that is host coherent, indicated with
  ///           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
  ///         - Call vkFlushMappedMemoryRanges to after writing to the mapped
  ///           memory, and call vkInvalidateMappedMemoryRanges before reading
  ///           from the mapped memory.
  /// \note Mapping different regions may affect performance. Mapping the whole
  ///       memory once might be a better choice.
  /// \note There can exist only one mapped instance at a time. Calling this
  ///       method with a current map will generate an error.
  /// \param size_in_bytes [def=0]   Region size in bytes to be mapped. If 0
  ///                                than the full available range is mapped.
  /// \param offset        [def=0]   Mapped memory offset in bytes.
  /// \param flags         [def={}]  Mapping flags.
  /// \return pointer to mapped memory starting at offset.
  HERMES_NODISCARD Result<void *> map(VkDeviceSize size_in_bytes = 0,
                                      VkDeviceSize offset = 0,
                                      VkMemoryMapFlags flags = {}) const;
  /// Flush a memory range to make it visible to the device. (Required for
  /// non-coherent memory)
  ///\param size_in_bytes [def=VK_WHOLE_SIZE] Size of the memory range to flush.
  ///\param offset        [def=0]             Byte offset from beginning.
  ///\return error status.
  VeResult flush(VkDeviceSize size_in_bytes = VK_WHOLE_SIZE,
                 VkDeviceSize offset = 0);
  /// Invalidate a memory range to make it visible to the host (Required for
  /// non-coherent memory)
  ///\param size_in_bytes [def=VK_WHOLE_SIZE] Size of the memory range to
  ///                                         invalidate.
  ///\param offset        [def=0]             Byte offset from beginning.
  ///\return error status.
  VeResult invalidate(VkDeviceSize size_in_bytes = VK_WHOLE_SIZE,
                      VkDeviceSize offset = 0);
  /// Unmap mapped memory.
  /// \note This invalidates the memory pointed by mapped().
  void unmap() const;

  /// \brief Copies data into this memory.
  /// \param data pointer
  /// \param size_in_bytes
  /// \param offset [def=0] Offset (in bytes) of the destination copy location.
  /// \param flags [def={}] Mapping flags.
  /// \return Error status.
  HERMES_NODISCARD VeResult copy(const void *data, VkDeviceSize size_in_bytes,
                                 VkDeviceSize offset = 0,
                                 VkMemoryMapFlags flags = {});
  /// Frees device memory and destroys this object.
  void destroy() noexcept;
  void swap(DeviceMemory &rhs) noexcept;
  /// \return This memory capacity in bytes.
  VkDeviceSize size() const;

protected:
  VmaAllocator vma_allocator_{VK_NULL_HANDLE};
  VmaAllocation vma_allocation_{VK_NULL_HANDLE};
  mutable void *mapped_{nullptr};

private:
#ifdef VENUS_DEBUG
  Config config_;
#endif

  VENUS_TO_STRING_FRIEND(DeviceMemory);
};

} // namespace venus::mem
