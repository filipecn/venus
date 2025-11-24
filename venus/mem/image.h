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
  /// \tparam Derived return type of configuration methods.
  /// \tparam Type type of the object build by this setup.
  template <typename Derived, typename Type> struct Setup {
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
    static Derived defaults(const VkExtent2D &extent,
                            VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);
    static Derived defaults(const VkExtent3D &extent,
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
    static Derived forDepthBuffer(const VkExtent2D &extent,
                                  VkFormat format = VK_FORMAT_D16_UNORM);

    /// \param info Image create info.
    Derived &setInfo(VkImageCreateInfo info);
    Derived &addCreateFlags(VkImageCreateFlags flags);
    Derived &setImageType(VkImageType image_type);
    Derived &setFormat(VkFormat format);
    Derived &setExtent(VkExtent3D extent);
    Derived &setMipLevels(u32 mipLevels);
    Derived &setArrayLayers(u32 array_layers);
    Derived &setSamples(VkSampleCountFlagBits samples);
    Derived &setTiling(VkImageTiling tiling);
    Derived &addUsage(VkImageUsageFlags usage);
    Derived &setSharingMode(VkSharingMode sharing_mode);
    Derived &addQueueFamilyIndex(u32 queue_family_index_count);
    Derived &setInitialLayout(VkImageLayout initial_layout);
    /// \param aspect
    Derived &addAspectMask(VkImageAspectFlags aspect);
    /// \param features to append
    Derived &addFormatFeatures(VkFormatFeatureFlagBits features);
    //
    VkImageCreateInfo createInfo() const;
    /// Create image from this configuration.
    /// \param device
    /// \return image or error.
    HERMES_NODISCARD Result<Type> build(const core::Device &device) const;
    /// \brief Creates an image from an already existent image.
    /// \note The newly created image gains ownership over the given vk_image
    ///       and will destroy it with destroy() is called.
    HERMES_NODISCARD Result<Type> build(VkDevice vk_device,
                                        VkImage vk_image) const;
    /// Create an initialized image from this configuration.
    /// \note This copies data into image's buffer memory.
    /// \param gd Graphics device with access to a command buffer.
    /// \param data Image contents.
    // HERMES_NODISCARD Result<Image> createAndUpload(GraphicsDevice &gd,
    //                                                const void *data) const;

  protected:
    std::optional<VmaAllocationCreateInfo> allocation_;
    // format
    VkFormatFeatureFlags format_features_;
    // image
    VkImageAspectFlags aspect_mask_;
    VkImageCreateInfo info_;
    std::vector<u32> queue_family_indices_;
  };
  struct Config : public Setup<Config, Image> {
    VENUS_to_string_FRIEND(Image::Config);
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

      Result<View> build(const Image &image) const;

    private:
      VkImageViewCreateInfo info_{};

      VENUS_to_string_FRIEND(Image::View::Config);
    };

    // raii

    VENUS_DECLARE_RAII_FUNCTIONS(View);

    void destroy() noexcept;
    void swap(View &rhs) noexcept;
    VkImageView operator*() const;
    operator bool() const;

  private:
    VkDevice vk_device_{VK_NULL_HANDLE};
    VkImageView vk_image_view_{VK_NULL_HANDLE};
#ifdef VENUS_DEBUG
    Config config_{};
#endif

    VENUS_to_string_FRIEND(Image::View);
  };
  /// Holds image object vk handles
  struct Handle {
    VkImage image;
    VkImageView view;
    VENUS_to_string_FRIEND(Image::Handle);
  };

  VENUS_DECLARE_RAII_FUNCTIONS(Image);

  /// Frees memory and destroy image/memory objects.
  virtual void destroy() noexcept;
  void swap(Image &rhs) noexcept;
  /// \return Underlying image vulkan object.
  VkImage operator*() const;
  operator bool() const;
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

  VENUS_to_string_FRIEND(Image);
};

template <typename Derived, typename Type>
Derived Image::Setup<Derived, Type>::defaults(const VkExtent3D &extent,
                                              VkFormat format) {
  return Image::Setup<Derived, Type>()
      .addCreateFlags({})
      .setImageType(VK_IMAGE_TYPE_2D)
      .setFormat(format)
      .setExtent(extent)
      .setMipLevels(1)
      .setArrayLayers(1)
      .setSamples(VK_SAMPLE_COUNT_1_BIT)
      .setTiling(VK_IMAGE_TILING_LINEAR)
      .addUsage(VK_IMAGE_USAGE_SAMPLED_BIT)
      .setSharingMode(VK_SHARING_MODE_EXCLUSIVE)
      .setInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
      .addAspectMask(VK_IMAGE_ASPECT_COLOR_BIT);
}

template <typename Derived, typename Type>
Derived Image::Setup<Derived, Type>::defaults(const VkExtent2D &extent,
                                              VkFormat format) {
  return Image::Config::defaults(VkExtent3D(extent.width, extent.height, 1),
                                 format);
}

template <typename Derived, typename Type>
Derived Image::Setup<Derived, Type>::forDepthBuffer(const VkExtent2D &extent,
                                                    VkFormat format) {
  return Image::Config()
      .addCreateFlags({})
      .setImageType(VK_IMAGE_TYPE_2D)
      .setFormat(format)
      .setExtent(VkExtent3D(extent.width, extent.height, 1))
      .setMipLevels(1)
      .setArrayLayers(1)
      .setSamples(VK_SAMPLE_COUNT_1_BIT)
      .setTiling(VK_IMAGE_TILING_OPTIMAL)
      .addUsage(VK_IMAGE_USAGE_SAMPLED_BIT |
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
      .setSharingMode(VK_SHARING_MODE_EXCLUSIVE)
      .setInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
      .addAspectMask(VK_IMAGE_ASPECT_DEPTH_BIT);
}

template <typename Derived, typename Type>
VkImageCreateInfo Image::Setup<Derived, Type>::createInfo() const {
  auto info = info_;
  info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  info.pQueueFamilyIndices = queue_family_indices_.data();
  info.queueFamilyIndexCount = queue_family_indices_.size();
  return info;
}

VENUS_DEFINE_SETUP_SET_FIELD_METHOD(Image, setInfo, VkImageCreateInfo, info_)
VENUS_DEFINE_SETUP_SET_FIELD_METHOD(Image, addCreateFlags, VkImageCreateFlags,
                                    info_.flags)
VENUS_DEFINE_SETUP_SET_FIELD_METHOD(Image, setImageType, VkImageType,
                                    info_.imageType)
VENUS_DEFINE_SETUP_SET_FIELD_METHOD(Image, setFormat, VkFormat, info_.format)
VENUS_DEFINE_SETUP_SET_FIELD_METHOD(Image, setExtent, VkExtent3D, info_.extent)
VENUS_DEFINE_SETUP_SET_FIELD_METHOD(Image, setMipLevels, u32, info_.mipLevels)
VENUS_DEFINE_SETUP_SET_FIELD_METHOD(Image, setArrayLayers, u32,
                                    info_.arrayLayers)
VENUS_DEFINE_SETUP_SET_FIELD_METHOD(Image, setSamples, VkSampleCountFlagBits,
                                    info_.samples)
VENUS_DEFINE_SETUP_SET_FIELD_METHOD(Image, setTiling, VkImageTiling,
                                    info_.tiling)
VENUS_DEFINE_SETUP_ADD_FLAGS_METHOD(Image, addUsage, VkImageUsageFlags,
                                    info_.usage)
VENUS_DEFINE_SETUP_SET_FIELD_METHOD(Image, setSharingMode, VkSharingMode,
                                    info_.sharingMode)
VENUS_DEFINE_SETUP_METHOD(Image, addQueueFamilyIndex, u32,
                          queue_family_indices_.emplace_back(value))
VENUS_DEFINE_SETUP_SET_FIELD_METHOD(Image, setInitialLayout, VkImageLayout,
                                    info_.initialLayout)
VENUS_DEFINE_SETUP_ADD_FLAGS_METHOD(Image, addAspectMask, VkImageAspectFlags,
                                    aspect_mask_)
VENUS_DEFINE_SETUP_ADD_FLAGS_METHOD(Image, addFormatFeatures,
                                    VkFormatFeatureFlagBits, format_features_)

/// \brief Holds a self-allocated vulkan image object.
/// The AllocatedImage owns the device memory used by the buffer.
class AllocatedImage : public Image, public DeviceMemory {
public:
  struct Config : public Image::Setup<Config, AllocatedImage>,
                  public DeviceMemory::Setup<Config, AllocatedImage> {
    Config &setImageConfig(const Image::Config &config);
    Config &setMemoryConfig(const DeviceMemory::Config &config);

    Result<AllocatedImage> build(const core::Device &device) const;

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
  VENUS_to_string_FRIEND(AllocatedImage);
};

/// The image pool holds an set of images and image views that can be accessed
/// by through unique names.
class ImagePool {
public:
  VENUS_DECLARE_RAII_FUNCTIONS(ImagePool)

  void destroy() noexcept;
  void swap(ImagePool &rhs);

  /// Creates a new image with label name.
  /// \param name Image label.
  /// \param config Allocated image config.
  /// \return error.
  template <class... P>
  HERMES_NODISCARD VeResult addImage(const std::string &name,
                                     const AllocatedImage::Config &config,
                                     P &&...params) {
    ImageData data{};
    VENUS_ASSIGN_OR_RETURN_BAD_RESULT(data.image,
                                      config.build(std::forward<P>(params)...));
    images_[name] = std::move(data);
    return VeResult::noError();
  }

  HERMES_NODISCARD VeResult
  addImageView(const std::string &image_name,
               const Image::View::Config &image_view_config);

  /// Gets image vulkan object.
  /// \param name Image key (label).
  /// \return Image or error.
  HERMES_NODISCARD Result<VkImage> operator[](const std::string &name) const;

private:
  struct ImageData {
    AllocatedImage image;
    Image::View view;
  };

  std::unordered_map<std::string, ImageData> images_;

  VENUS_to_string_FRIEND(ImagePool);
};

} // namespace venus::mem
