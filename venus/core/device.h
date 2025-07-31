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

/// \file   device.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30
/// \brief  Vulkan Logical Device

#pragma once

#include <venus/core/physical_device.h>

namespace venus::core {

/// The logical device makes the interface of the application and the physical
/// device. Represents the hardware, along with the extensions and features
/// enabled for it and all the queues requested from it. The logical device
/// allows us to record commands, submit them to queues and acquire the results.
/// Device queues need to be requested on device creation, we cannot create or
/// destroy queues explicitly. They are created/destroyed on logical device
/// creation/destruction.
class Device {
public:
  /// Builder for Device class.
  struct Config {
    VkPhysicalDeviceFeatures f;
    VkPhysicalDeviceFeatures2 f2;
    VkPhysicalDeviceVulkan13Features f13{};
    VkPhysicalDeviceVulkan12Features f12{};
    VkPhysicalDeviceDescriptorIndexingFeaturesEXT desc_i_f;
    VkPhysicalDeviceSynchronization2FeaturesKHR sync2f;
  };
  Device() noexcept;

private:
};

class GraphicsDevice : public Device {};

} // namespace venus::core
