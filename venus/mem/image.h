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

/// \file   image.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30
/// \brief  Vulkan image.

#pragma once

#include <venus/mem/device_memory.h>

namespace venus::mem {

/// A image is a region of memory that can be used to store data that can be
/// accessed by the CPU and the GPU. The image is created on the device memory,
/// and it can be mapped to the CPU memory. Images are used by textures and
/// accessed by samplers in shaders.
/// \note RAII
class Image {
public:
  /// Builder for image.
  struct Config {
    friend class Image;
    /// \brief Creates defaults config for images.
    /// Default values are:
    ///   - ImageType = e2D
    ///   - MipLevels = 1
    ///   - ArrayLayers = 1
    ///   - Samples = e1
    ///   - Tiling = eLinear
    ///   - Usage = eSampled
    ///   - SharingMode = eExclusive
    ///   - InitialLayout = eUndefined
    ///   - aspect mask = eColor;
    ///   - memory properties = eDeviceLocal
    /// \param extent Image extent in pixels.
    /// \param format [def=eR8G8B8A8Unorm] Image format.
    static Config defaults(const VkExtent2D &extent,
                           VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);
    /// \brief Creates defaults config for images.
    /// Default values are:
    ///   - ImageType = e2D
    ///   - MipLevels = 1
    ///   - ArrayLayers = 1
    ///   - Samples = e1
    ///   - Tiling = eOptimal
    ///   - Usage = eDepthStencilAttachment | eSampled
    ///   - SharingMode = eExclusive
    ///   - InitialLayout = eUndefined
    ///   - aspect mask = eDepth;
    ///   - memory properties = eDeviceLocal
    /// \param extent Image extent in pixels.
    /// \param format [def=eD16Unorm] Image format.
    static Config forDepthBuffer(const VkExtent2D &extent,
                                 VkFormat format = VK_FORMAT_D16_UNORM);

    /// \param info Image create info.
    Config &setInfo(VkImageCreateInfo info);
    Config &addCreateFlags(VkImageCreateFlags flags);
    Config &setImageType(VkImageType image_type);
    Config &setFormat(VkFormat format);
    Config &setExtent(VkExtent3D extent);
    Config &setMipLevels(u32 mipLevels);
    Config &setArrayLayers(u32 array_layers);
    Config &setSamples(VkSampleCountFlagBits samples);
    Config &setTiling(VkImageTiling tiling);
    Config &addUsage(VkImageUsageFlags usage);
    Config &setSharingMode(VkSharingMode sharing_mode);
    Config &addQueueFamilyIndex(u32 queue_family_index_count);
    Config &setInitialLayout(VkImageLayout initial_layout);
    /// \param aspect
    Config &addAspectMask(VkImageAspectFlags aspect);
    /// \param features to append
    Config &addFormatFeatures(VkFormatFeatureFlagBits features);
    //
    VkImageCreateInfo createInfo() const;
    /// Create image from this configuration.
    /// \param device
    /// \return image or error.
    HERMES_NODISCARD Result<Image> create(const core::Device &device) const;
    /// \brief Creates an image from an already existent image.
    /// \note The newly created image gains ownership over the given vk_image
    ///       and will destroy it with destroy() is called.
    HERMES_NODISCARD Result<Image> create(VkDevice vk_device,
                                          VkImage vk_image) const;
    /// Create an initialized image from this configuration.
    /// \note This copies data into image's buffer memory.
    /// \param gd Graphics device with access to a command buffer.
    /// \param data Image contents.
    // HERMES_NODISCARD Result<Image> createAndUpload(GraphicsDevice &gd,
    //                                                const void *data) const;

  private:
    std::optional<VmaAllocationCreateInfo> allocation_;
    // format
    VkFormatFeatureFlags format_features_;
    // image
    VkImageAspectFlags aspect_mask_;
    VkImageCreateInfo info_;
    std::vector<u32> queue_family_indices_;
    // memory
    DeviceMemory::Config memory_;

    VENUS_TO_STRING_FRIEND(Image::Config);
  };
  /// Image views allow us to define how image's memory is accessed and
  /// interpreted. For example, we can choose to look at the buffer as a uniform
  /// texel buffer or as a storage texel buffer.
  /// \note this uses raii.
  class View final {
    friend class Image;

  public:
    /// Builder for Image::View
    struct Config {
      Config &setFlags(VkImageViewCreateFlags flags);
      Config &setImage(VkImage image);
      Config &setViewType(VkImageViewType view_type);
      Config &setFormat(VkFormat format);
      Config &setComponents(VkComponentMapping components);
      Config &setSubresourceRange(VkImageSubresourceRange subresource_range);

      Result<View> create(const Image &image) const;

    private:
      VkImageViewCreateInfo info_{};

      VENUS_TO_STRING_FRIEND(Image::View::Config);
    };

    // raii

    VENUS_DECLARE_RAII_FUNCTIONS(View);

    void destroy() noexcept;
    void swap(View &rhs) noexcept;
    VkImageView operator*() const;

  private:
    VkDevice vk_device_{VK_NULL_HANDLE};
    VkImageView vk_image_view_{VK_NULL_HANDLE};
#ifdef VENUS_DEBUG
    Config config_{};
#endif

    VENUS_TO_STRING_FRIEND(Image::View);
  };

  VENUS_DECLARE_RAII_FUNCTIONS(Image);

  /// Frees memory and destroy image/memory objects.
  virtual void destroy() noexcept;
  void swap(Image &rhs) noexcept;
  /// \return Underlying image vulkan object.
  VkImage operator*() const;
  /// \return Image data format.
  VkFormat format() const;
  /// \return Associated device.
  VkDevice device() const;

protected:
  VkImage vk_image_{VK_NULL_HANDLE};
  VkDevice vk_device_{VK_NULL_HANDLE};
  VkFormat vk_format_;

private:
  bool ownership_{true};
#ifdef VENUS_DEBUG
  Config config_;
#endif

  VENUS_TO_STRING_FRIEND(Image);
};

/// \brief Holds a self-allocated vulkan image object.
/// The AllocatedImage owns the device memory used by the buffer.
class AllocatedImage : public Image, public DeviceMemory {
public:
  struct Config {
    Config &setImageConfig(const Image::Config &config);
    Config &setMemoryConfig(const DeviceMemory::Config &config);

    Result<AllocatedImage> create(const core::Device &device) const;

  private:
    Image::Config image_config_;
    DeviceMemory::Config mem_config_;
  };

  VENUS_DECLARE_RAII_FUNCTIONS(AllocatedImage)

  /// Frees memory and destroy buffer/memory objects.
  void destroy() noexcept override;
  //
  void swap(AllocatedImage &rhs) noexcept;

private:
  VENUS_TO_STRING_FRIEND(AllocatedImage);
};

} // namespace venus::mem
