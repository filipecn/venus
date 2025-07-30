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

/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-06-07
/// \brief  Example on how to initialize venus.

#include "venus/core/debug.h"
#include <venus/core/instance.h>

int main() {
  using namespace venus::core;
  // venus::core::SystemTime::init();
  VENUS_CHECK_VE_RESULT(vk::init());
  Instance instance;
  VENUS_ASSIGN_RESULT(instance, Instance::Config()
                                    .setApiVersion(vk::Version(1, 4, 0))
                                    .setName("hello_vulkan_app")
                                    .enableDebugUtilsExtension()
                                    .create());

  HERMES_INFO("\n{}", venus::to_string(instance));
  std::vector<PhysicalDevice> physical_devices;
  VENUS_ASSIGN_RESULT(physical_devices,
                      instance.enumerateAvailablePhysicalDevices());
  for (const auto &d : physical_devices)
    HERMES_INFO("\n{}", venus::to_string(d));
  // venus::core::Instance instance("hello_vulkan_app", {}, {});
  // auto devices = instance.enumerateAvailablePhysicalDevices();
  // td::cout << devices.size() << std::endl;
  return 0;
}
