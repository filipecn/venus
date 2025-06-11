/// Copyright (c) 2025, FilipeCN.
///
/// The MIT License (MIT)
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///
/// \file debug.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date 2025-06-07
///
/// \brief Auxiliary functions for debugging and error handling
///
#pragma once

#include <cassert>
#include <hermes/common/debug.h>
#include <sstream>
#include <string>
#include <venus/core/vk.h>

#ifndef VENUS_UNUSED_VARIABLE
#define VENUS_UNUSED_VARIABLE(x) ((void)x);
#endif
//
// #ifndef VENUS_ASSERT
// #define VENUS_ASSERT(exp, ...) HERMES_ASSERT(exp __VA_OPT__(, ) __VA_ARGS__)
// #endif
//
// #ifndef VENUS_CHECK
// #define VENUS_CHECK(exp, ...) HERMES_CHECK(exp __VA_OPT__(, ) __VA_ARGS__)
// #endif

#ifdef NDEBUG

#define ASSERT(exp)
#define VENUS_DEBUG(...)
#define VENUS_TRACE(...)
#define VENUS_INFO(...)
#define VENUS_WARN(...)
#define VENUS_ERROR(...)
#define VENUS_CRITICAL(...)

#else

#ifndef VENUS_DEBUG
#define VENUS_DEBUG(FMT, ...) HERMES_DEBUG(FMT __VA_OPT__(, ) __VA_ARGS__)
#endif

#ifndef VENUS_TRACE
#define VENUS_TRACE(FMT, ...) HERMES_TRACE(FMT __VA_OPT__(, ) __VA_ARGS__)
#endif

#ifndef VENUS_INFO
#define VENUS_INFO(FMT, ...) HERMES_INFO(FMT __VA_OPT__(, ) __VA_ARGS__)
#endif

#ifndef VENUS_WARN
#define VENUS_WARN(FMT, ...) HERMES_WARN(FMT __VA_OPT__(, ) __VA_ARGS__)
#endif

#ifndef VENUS_ERROR
#define VENUS_ERROR(FMT, ...) HERMES_ERROR(FMT __VA_OPT__(, ) __VA_ARGS__)
#endif

#ifndef VENUS_CRITICAL
#define VENUS_CRITICAL(FMT, ...) HERMES_CRITICAL(FMT __VA_OPT__(, ) __VA_ARGS__)
#endif

#endif // NDEBUG

/// Retrieves the description of VkResult values
/// \param err **[in]** error code
/// \return std::string error description
inline std::string vulkanResultString(VkResult err) {
  switch (err) {
  case VK_SUCCESS:
    return "VK_SUCCESS Command successfully completed";
  case VK_NOT_READY:
    return "VK_NOT_READY A fence or query has not yet completed";
  case VK_TIMEOUT:
    return "VK_TIMEOUT A wait operation has not completed in the "
           "specified "
           "time";
  case VK_EVENT_SET:
    return "VK_EVENT_SET An event is signaled";
  case VK_EVENT_RESET:
    return "VK_EVENT_RESET An event is unsignaled";
  case VK_INCOMPLETE:
    return "VK_INCOMPLETE A return array was too small for the "
           "result";
  case VK_SUBOPTIMAL_KHR:
    return "VK_SUBOPTIMAL_KHR A swapchain no longer matches the "
           "surface "
           "properties exactly, but can still be used to present to "
           "the "
           "surface successfully.";
  case VK_ERROR_OUT_OF_HOST_MEMORY:
    return "VK_ERROR_OUT_OF_HOST_MEMORY A host memory allocation has "
           "failed.";
  case VK_ERROR_OUT_OF_DEVICE_MEMORY:
    return "VK_ERROR_OUT_OF_DEVICE_MEMORY A device memory allocation "
           "has "
           "failed.";
  case VK_ERROR_INITIALIZATION_FAILED:
    return "VK_ERROR_INITIALIZATION_FAILED Initialization of an "
           "object could "
           "not be completed for implementation-specific reasons.";
  case VK_ERROR_DEVICE_LOST:
    return "VK_ERROR_DEVICE_LOST The logical or physical device has "
           "been "
           "lost. ";
  case VK_ERROR_MEMORY_MAP_FAILED:
    return "VK_ERROR_MEMORY_MAP_FAILED Mapping of a memory object "
           "has failed.";
  case VK_ERROR_LAYER_NOT_PRESENT:
    return "VK_ERROR_LAYER_NOT_PRESENT A requested layer is not "
           "present or "
           "could not be loaded.";
  case VK_ERROR_EXTENSION_NOT_PRESENT:
    return "VK_ERROR_EXTENSION_NOT_PRESENT A requested extension is "
           "not "
           "supported.";
  case VK_ERROR_FEATURE_NOT_PRESENT:
    return "VK_ERROR_FEATURE_NOT_PRESENT A requested feature is not "
           "supported.";
  case VK_ERROR_INCOMPATIBLE_DRIVER:
    return "VK_ERROR_INCOMPATIBLE_DRIVER The requested version of "
           "Vulkan is "
           "not supported by the driver or is otherwise incompatible "
           "for "
           "implementation-specific reasons.";
  case VK_ERROR_TOO_MANY_OBJECTS:
    return "VK_ERROR_TOO_MANY_OBJECTS Too many objects of the type "
           "have "
           "already been created.";
  case VK_ERROR_FORMAT_NOT_SUPPORTED:
    return "VK_ERROR_FORMAT_NOT_SUPPORTED A requested format is not "
           "supported "
           "on this device.";
  case VK_ERROR_FRAGMENTED_POOL:
    return "VK_ERROR_FRAGMENTED_POOL A pool allocation has failed "
           "due to "
           "fragmentation of the pool’s memory. This must only be "
           "returned if "
           "no attempt to allocate host or device memory was made to "
           "accommodate the new allocation. This should be returned "
           "in "
           "preference to VK_ERROR_OUT_OF_POOL_MEMORY, but only if "
           "the "
           "implementation is certain that the pool allocation "
           "failure was due "
           "to fragmentation.";
  case VK_ERROR_SURFACE_LOST_KHR:
    return "VK_ERROR_SURFACE_LOST_KHR A surface is no longer "
           "available.";
  case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
    return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR The requested window "
           "is already "
           "in use by Vulkan or another API in a manner which "
           "prevents it from "
           "being used again.";
  case VK_ERROR_OUT_OF_DATE_KHR:
    return "VK_ERROR_OUT_OF_DATE_KHR A surface has changed in such a "
           "way that "
           "it is no longer compatible with the swapchain, and "
           "further "
           "presentation requests using the swapchain will fail. "
           "Applications "
           "must query the new surface properties and recreate their "
           "swapchain "
           "if they wish to continue presenting to the surface.";
  case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
    return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR The display used by a "
           "swapchain "
           "does not use the same presentable image layout, or is "
           "incompatible "
           "in a way that prevents sharing an image.";
  case VK_ERROR_INVALID_SHADER_NV:
    return "VK_ERROR_INVALID_SHADER_NV One or more shaders failed to "
           "compile "
           "or link. More details are reported back to the "
           "application via "
           "https://www.khronos.org/registry/vulkan/specs/"
           "1.1-extensions/html/"
           "vkspec.html#VK_EXT_debug_report if enabled.";
#ifndef __linux
  case VK_ERROR_OUT_OF_POOL_MEMORY:
    return "VK_ERROR_OUT_OF_POOL_MEMORY A pool memory allocation has "
           "failed. "
           "This must only be returned if no attempt to allocate "
           "host or "
           "device memory was made to accommodate the new "
           "allocation. If the "
           "failure was definitely due to fragmentation of the pool, "
           "VK_ERROR_FRAGMENTED_POOL should be returned instead.";
  case VK_ERROR_INVALID_EXTERNAL_HANDLE:
    return "VK_ERROR_INVALID_EXTERNAL_HANDLE An external handle is "
           "not a valid "
           "handle of the specified type.";
  case VK_ERROR_FRAGMENTATION_EXT:
    return "VK_ERROR_FRAGMENTATION_EXT A descriptor pool creation "
           "has failed "
           "due to fragmentation.";
#ifndef WIN32
  case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
    return "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT A buffer creation "
           "failed "
           "because the requested address is not available.";
  case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
    return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT An "
           "operation on a "
           "swapchain created with "
           "VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT "
           "failed as it "
           "did not have exlusive full-screen access. This may occur "
           "due to "
           "implementation-dependent reasons, outside of the "
           "application’s "
           "control.";
  // case VK_ERROR_OUT_OF_POOL_MEMORY_KHR:
  //   return "VK_ERROR_OUT_OF_POOL_MEMORY_KHR";
  case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
    return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
#endif
  case VK_ERROR_NOT_PERMITTED_EXT:
    return "VK_ERROR_NOT_PERMITTED_EXT";
#endif
  case VK_ERROR_VALIDATION_FAILED_EXT:
    return "VK_ERROR_VALIDATION_FAILED_EXT";
    // case VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR:
    //   return "VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR";
  default:
    return "UNDEFINED";
  }
  return "UNDEFINED";
}

//#define CHECK_VULKAN(A)                                                        \
//  {                                                                            \
//    VkResult err = (A);                                                        \
//    if (err != VK_SUCCESS) {                                                   \
//      VENUS_ERROR("{}", #A);                                                   \
//      VENUS_ERROR("{}", vulkanResultString(err));                              \
//    }                                                                          \
//  }
/////
// #d ef in e R_CHECK_VULKAN(A, R) \
//  { \
//    VkResult err = (A); \
//    if (err != VK_SUCCESS) { \
//      VENUS_ERROR("{}", #A); \
//      VENUS_ERROR("{}", vulkanResultString(err)); \
//      return R; \
//    } \
//  }
/////
// #d ef in e ASSERT_VULKAN(A) \
//  { \
//    VkResult err = (A); \
//    if (err != VK_SUCCESS) { \
//      VENUS_ERROR(#A); \
//      VENUS_ERROR(vulkanResultString(err)); \
//      exit(-1); \
//    } \
//  }
