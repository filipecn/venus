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
  /// Builder for DeviceMemory.
  /// \tparam Derived return type of configuration methods.
  /// \tparam Type type of the object build by this setup.
  template <typename Derived, typename Type> struct Setup {
    /// This sets:
    ///  - device local
    ///  - gpu usage only
    static Derived forTexture();
    /// \param flags allocation flags (vma)
    Derived &setAllocationFlags(VmaAllocationCreateFlags flags);
    /// \param usage memory usage (vma)
    Derived &setMemoryUsage(VmaMemoryUsage usage);
    /// \param properties appending properties.
    Derived &addRequiredProperties(VkMemoryPropertyFlags properties);
    /// \param properties appending properties.
    Derived &addPreferredProperties(VkMemoryPropertyFlags properties);
    /// \param Bitmask containing one bit set for every memory type acceptable
    ///        for this allocation.
    Derived &addMemoryType(u32 type_bits);
    /// \param pool (vma)
    Derived &setPool(VmaPool pool);
    /// \param priority value in [0,1]
    Derived &setPriority(f32 priority);
    /// \param requirements new memory requirements.
    Derived &setMemoryRequirements(const VkMemoryRequirements &requirements);
    /// Set memory host visible.
    /// \note This sets memory properties eHostCoherent and eHostVisible.
    Derived &setHostVisible();
    /// Set memory host visible.
    /// \note This sets memory properties eDeviceLocal.
    Derived &setDeviceLocal();
    /// Creates a new DeviceMemory from this configuration.
    /// \param device Device holding the new allocated memory.
    /// \return a new DeviceMemory, or error.
    HERMES_NODISCARD Result<Type> build(const core::Device &device) const;

  protected:
    VkMemoryRequirements requirements_{};
    VmaAllocationCreateInfo vma_allocation_create_info_{};
  };

  struct Config : public Setup<Config, DeviceMemory> {
    VENUS_to_string_FRIEND(DeviceMemory::Config);
  };

  class ScopedMap {
  public:
    ~ScopedMap() noexcept;

    template <typename T> T *get() { return reinterpret_cast<T *>(mapped_); }

  private:
    ScopedMap(DeviceMemory &memory, void *mapped) noexcept;

    DeviceMemory &memory_;
    void *mapped_{nullptr};

    friend class DeviceMemory;
  };

  // raii

  VENUS_DECLARE_RAII_FUNCTIONS(DeviceMemory)

  /// \return Scoped map object with access of this memory.
  HERMES_NODISCARD Result<ScopedMap> scopedMap();
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
  /// \return pointer to mapped memory.
  HERMES_NODISCARD Result<void *> map() const;
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
  /// \return access status.
  HERMES_NODISCARD VeResult access(const std::function<void(void *)> &f) const;
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

  VENUS_to_string_FRIEND(DeviceMemory);
};

template <typename Derived, typename Type>
Derived DeviceMemory::Setup<Derived, Type>::forTexture() {
  return DeviceMemory::Setup<Derived, Type>().setDeviceLocal().setMemoryUsage(
      VMA_MEMORY_USAGE_GPU_ONLY);
}

template <typename Derived, typename Type>
Derived &DeviceMemory::Setup<Derived, Type>::setHostVisible() {
  vma_allocation_create_info_.requiredFlags |=
      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
  return static_cast<Derived &>(*this);
}

template <typename Derived, typename Type>
Derived &DeviceMemory::Setup<Derived, Type>::setDeviceLocal() {
  vma_allocation_create_info_.requiredFlags |=
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  return static_cast<Derived &>(*this);
}

VENUS_DEFINE_SETUP_SET_FIELD_METHOD(DeviceMemory, setMemoryUsage,
                                    VmaMemoryUsage,
                                    vma_allocation_create_info_.usage);
VENUS_DEFINE_SETUP_ADD_FLAGS_METHOD(DeviceMemory, setAllocationFlags,
                                    VmaAllocationCreateFlags,
                                    vma_allocation_create_info_.flags);
VENUS_DEFINE_SETUP_ADD_FLAGS_METHOD(DeviceMemory, addRequiredProperties,
                                    VkMemoryPropertyFlags,
                                    vma_allocation_create_info_.requiredFlags);
VENUS_DEFINE_SETUP_ADD_FLAGS_METHOD(DeviceMemory, addPreferredProperties,
                                    VkMemoryPropertyFlags,
                                    vma_allocation_create_info_.preferredFlags);
VENUS_DEFINE_SETUP_ADD_FLAGS_METHOD(DeviceMemory, addMemoryType, u32,
                                    vma_allocation_create_info_.memoryTypeBits);
VENUS_DEFINE_SETUP_SET_FIELD_METHOD(DeviceMemory, setPool, VmaPool,
                                    vma_allocation_create_info_.pool);
VENUS_DEFINE_SETUP_SET_FIELD_METHOD(DeviceMemory, setPriority, f32,
                                    vma_allocation_create_info_.priority);
VENUS_DEFINE_SETUP_SET_FIELD_METHOD(DeviceMemory, setMemoryRequirements,
                                    const VkMemoryRequirements &,
                                    requirements_);

} // namespace venus::mem
