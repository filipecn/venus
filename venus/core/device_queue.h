/* Copyright (c) 2026, FilipeCN.
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

/// \file   device_queue.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2026-02-12

#include <venus/utils/vk_debug.h>

namespace venus::core {

class DeviceQueue {
public:
private:
  VkQueue vk_queue_{VK_NULL_HANDLE};
  VkDevice vk_device_{VK_NULL_HANDLE};

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
  friend struct hermes::DebugTraits<DeviceQueue>;
#endif
};

} // namespace venus::core

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
namespace hermes {

template <> struct DebugTraits<venus::core::DeviceQueue> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage message(const venus::core::DeviceQueue &data) {
    return DebugMessage()
        .addTitle("Device Queue")
        .add("vk_queue", VENUS_VK_DISPATCHABLE_HANDLE_STRING(data.vk_queue_))
        .add("vk_device", VENUS_VK_DISPATCHABLE_HANDLE_STRING(data.vk_device_));
  }
};

} // namespace hermes

#endif // VENUS_INCLUDE_DEBUG_TRAITS