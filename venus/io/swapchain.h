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

/// \file   swapchain.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30
/// \brief  Vulkan Swapchain

#pragma once

#include <venus/core/device.h>

namespace venus::io {

/// \brief Controls a swap chain.
/// Different from other high level libraries, such as OpenGL, Vulkan does
/// not have a system of framebuffers. In order to control the buffers that
/// are rendered and presented on the display, Vulkan provides a mechanism
/// called swap chain. The Vulkan swap chain is a queue of images that are
/// presented to the screen in a synchronized manner, following the rules and
/// properties defined on its setup. The swap chain is owned by the
/// presentation engine, and not by the application. We can't create the
/// images or destroy them, all the application does is to request images, do
/// work and give it back to the presentation engine. In order to use the
/// swap chain, the device has to support the VK_KHR_swap chain extension. The
/// swap chain works following a presentation mode. The presentation mode
/// defines the format of an image, the number of images (double/triple
/// buffering), v-sync and etc. In other words, it defines how images are
/// displayed on screen. Vulkan provides 4 presentation modes:
///
/// 1. IMMEDIATE mode
///    The image to be presented immediately replaces the image that is being
///    displayed. Screen tearing may happen when using this mode.
///
/// 2. FIFO mode
///    When a image is presented, it is added to the queue. Images are
///    displayed on screen in sync with blanking periods (v-sync). This mode
///    is similar to OpenGL's buffer swap.
///
/// 3. (FIFO) RELAXED mode
///    Images are displayed with blanking periods only when are faster than
///    the refresh rate.
///
/// 4. MAILBOX mode (triple buffering)
///    There is a queue with just one element. An image waiting in this queue
///    is displayed in sync with blanking periods. When the application
///    presents an image, the new image replaces the one waiting in the
///    queue. So the displayed image is always the most recent available.
///
/// \note This class uses RAII
class Swapchain final {
public:
  /// Builder for Swapchain class.
  struct Config {
    /// \param output surface handle.
    Config &setSurface(VkSurfaceKHR surface);
    /// \param old_swap_chain handle.
    Config &setOldSwapchain(VkSwapchainKHR old_swapchain);
    /// \param usage_flags
    Config &addUsageFlags(VkImageUsageFlags usage_flags);
    /// \param extent desired swapchain image size.
    Config &setExtent(const VkExtent2D &extent);
    /// \param present_mode desired swapchain presentation mode.
    Config &setPresentMode(VkPresentModeKHR present_mode);
    /// \param queue_indices
    Config &setQueueFamilyIndices(
        const core::vk::GraphicsQueueFamilyIndices &family_indices);
    /// \param format desired swapchain image format.
    Config &setFormat(VkFormat format);
    /// \param color_space desired swapchain image color space.
    Config &setColorSpace(VkColorSpaceKHR color_space);
    /// \param image_count
    Config &setImageCount(u32 image_count);
    /// \param flags
    Config &addCreateFlags(VkSwapchainCreateFlagsKHR flags);

    HERMES_NODISCARD Result<Swapchain> create(const core::Device &device) const;

  private:
    VkSurfaceKHR surface_{VK_NULL_HANDLE};
    VkImageUsageFlags usage_flags_{};
    VkExtent2D extent_{};
    VkPresentModeKHR present_mode_{VK_PRESENT_MODE_FIFO_KHR};
    VkSurfaceFormatKHR surface_format_{VK_FORMAT_B8G8R8A8_UNORM,
                                       VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    core::vk::GraphicsQueueFamilyIndices family_indices_{};
    VkSwapchainKHR old_swapchain_{VK_NULL_HANDLE};
    u32 image_count_{3};
    VkSwapchainCreateFlagsKHR flags_{};

    VENUS_TO_STRING_FRIEND(Swapchain::Config);
  };

  Swapchain() noexcept = default;
  Swapchain(const Swapchain &rhs) = delete;
  Swapchain(Swapchain &&rhs) noexcept;
  ~Swapchain() noexcept;

  Swapchain &operator=(const Swapchain &rhs) = delete;
  Swapchain &operator=(Swapchain &&rhs) noexcept;

  /// Destroy underlying vulkan swapchain object and clear all data.
  void destroy() noexcept;
  /// \return Vulkan swapchain handle.
  VkSwapchainKHR operator*() const;
  /// \return Associated device.
  VkDevice device() const;

private:
  VkSwapchainKHR vk_swapchain_{VK_NULL_HANDLE};
  VkDevice vk_device_{VK_NULL_HANDLE};

#ifdef VENUS_DEBUG
  Config config_{};
#endif

  VENUS_TO_STRING_FRIEND(Swapchain);
};

} // namespace venus::io
