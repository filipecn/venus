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
#include <format>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <venus/core/types.h>
#include <venus/core/vk.h>

namespace venus::core {

struct term_colors {

  /// \brief Set of 256-terminal color codes
  /// \note Extracted from
  /// https://misc.flogisoft.com/bash/tip_colors_and_formatting

  // SET
  static const char bold[5];       //!< "\e[1m"
  static const char dim[5];        //!< "\e[2m"
  static const char underlined[5]; //!< "\e[4m"
  static const char blink[5];      //!< "\e[5m"
  static const char inverted[5];   //!< "\e[7m"
  static const char hidden[5];     //!< "\e[8m"
  // RESET
  static const char reset[5];            //!< "\e[0m"
  static const char reset_bold[6];       //!< "\e[21m"
  static const char reset_dim[6];        //!< "\e[22m"
  static const char reset_underlined[6]; //!< "\e[24m"
  static const char reset_blink[6];      //!< "\e[25m"
  static const char reset_inverted[6];   //!< "\e[27m"
  static const char reset_hidden[6];     //!< "\e[28m"
  // 8/16 Colors
  static const char default_color[6];            //!< "\e[39m"
  static const char black[6];                    //!< "\e[30m"
  static const char red[6];                      //!< "\e[31m"
  static const char green[6];                    //!< "\e[32m"
  static const char yellow[6];                   //!< "\e[33m"
  static const char blue[6];                     //!< "\e[34m"
  static const char magenta[6];                  //!< "\e[35m"
  static const char cyan[6];                     //!< "\e[36m"
  static const char light_gray[6];               //!< "\e[37m"
  static const char dark_gray[6];                //!< "\e[90m"
  static const char light_red[6];                //!< "\e[91m"
  static const char light_green[6];              //!< "\e[92m"
  static const char light_yellow[6];             //!< "\e[93m"
  static const char light_blue[6];               //!< "\e[94m"
  static const char light_magenta[6];            //!< "\e[95m"
  static const char light_cyan[6];               //!< "\e[96m"
  static const char white[6];                    //!< "\e[97m"
  static const char background_default_color[6]; //!< "\e[49m"
  static const char background_black[6];         //!< "\e[40m"
  static const char background_red[6];           //!< "\e[41m"
  static const char background_green[6];         //!< "\e[42m"
  static const char background_yellow[6];        //!< "\e[43m"
  static const char background_blue[6];          //!< "\e[44m"
  static const char background_magenta[6];       //!< "\e[45m"
  static const char background_cyan[6];          //!< "\e[46m"
  static const char background_light_gray[6];    //!< "\e[47m"
  static const char background_dark_gray[7];     //!< "\e[100m"
  static const char background_light_red[7];     //!< "\e[101m"
  static const char background_light_green[7];   //!< "\e[102m"
  static const char background_light_yellow[7];  //!< "\e[103m"
  static const char background_light_blue[7];    //!< "\e[104m"
  static const char background_light_magenta[7]; //!< "\e[105m"
  static const char background_light_cyan[7];    //!< "\e[106m"
  static const char background_white[7];         //!< "\e[107m"

  /// \brief Get 88/256 color code
  /// \param color_number
  /// \return
  inline static std::string color(u8 color_number) {
    return std::string("\e[38;5;") + std::to_string(color_number) + "m";
  }
  /// \brief Get 88/256 background color code
  /// \param color_number
  /// \return
  inline static std::string background_color(u8 color_number) {
    return std::string("\e[48;5;") + std::to_string(color_number) + "m";
  }
  /// \brief Combine two color codes
  /// \param a
  /// \param b
  /// \return
  inline static std::string combine(const std::string &a,
                                    const std::string &b) {
    return "\e[" + a.substr(2, a.size() - 3) + ";" + b.substr(2, b.size() - 3) +
           "m";
  }

  template <typename T> static std::string numberColor(T n) {
    return term_colors::color(static_cast<u8>(n));
  }

  inline static std::string threadColor(std::thread::id thread_id) {
    return term_colors::numberColor(std::hash<std::thread::id>()(thread_id));
  }

  inline static std::string random() {
    static u8 next = 0;
    next += 13;
    return color(next);
  }
};

struct Log {
  enum class Level {
    debug = 0,
    trace = 1,
    info = 2,
    warn = 3,
    error = 4,
    critical = 5,
    COUNT = 6
  };
  struct Location {
    const char *filename;
    int line;
  };

  // log config
  static std::size_t s_filename_len;
  static Log::Level s_level;
  static std::ostream *s_os;
  static u8 s_colors[static_cast<std::size_t>(Level::COUNT)];
  static bool s_use_colors;

  static void init();

  static void message(Log::Level level, Location location,
                      const std::string &s);
};

} // namespace venus::core

#ifndef VENUS_UNUSED_VARIABLE
#define VENUS_UNUSED_VARIABLE(x) ((void)x);
#endif

#ifndef VENUS_ASSERT_M
#define VENUS_ASSERT_M(exp, ...)                                               \
  {                                                                            \
    if (exp) {                                                                 \
    } else {                                                                   \
      venus::core::Log::message(                                               \
          venus::core::Log::Level::critical,                                   \
          venus::core::Log::Location{__FILE__, __LINE__},                      \
          std::format(__VA_ARGS__));                                           \
    }                                                                          \
  }
#endif

#ifndef VENUS_ASSERT
#define VENUS_ASSERT(exp)                                                      \
  {                                                                            \
    if (exp) {                                                                 \
    } else {                                                                   \
      venus::core::Log::message(                                               \
          venus::core::Log::Level::critical,                                   \
          venus::core::Log::Location{__FILE__, __LINE__}, "assert");           \
    }                                                                          \
  }
#endif

#ifndef VENUS_CHECK
#define VENUS_CHECK(exp, ...)                                                  \
  {                                                                            \
    if (exp) {                                                                 \
    } else {                                                                   \
      venus::core::Log::message(                                               \
          venus::core::Log::Level::warn,                                       \
          venus::core::Log::Location{__FILE__, __LINE__},                      \
          std::format(__VA_ARGS__));                                           \
    }                                                                          \
  }
#endif

#ifdef NDEBUG

#define ASSERT(exp)
#define VENUS_DEBUG(...)
#define VENUS_TRACE(...)
#define VENUS_INFO(...)
#define VENUS_WARN(...)
#define VENUS_ERROR(...)
#define VENUS_CRITICAL(...)

#else

/// \brief Concatenates multiple objects into a single string.
/// \tparam Args
/// \param args
/// \return a single string of the resulting concatenation
template <class... Args> static std::string concat(const Args &...args) {
  std::stringstream s;
  (s << ... << args);
  return s.str();
}

#ifndef VENUS_DEBUG_VAL
#define VENUS_DEBUG_VAL(VAL)                                                   \
  venus::core::Log::message(venus::core::Log::Level::debug,                    \
                            venus::core::Log::Location{__FILE__, __LINE__},    \
                            std::format("{:5} = {}", #VAL, VAL))
#endif

#ifndef VENUS_DEBUG
#define VENUS_DEBUG(...)                                                       \
  venus::core::Log::message(venus::core::Log::Level::debug,                    \
                            venus::core::Log::Location{__FILE__, __LINE__},    \
                            concat(__VA_ARGS__))
#endif

#ifndef VENUS_TRACE
#define VENUS_TRACE(...)                                                       \
  venus::core::Log::message(venus::core::Log::Level::trace,                    \
                            venus::core::Log::Location{__FILE__, __LINE__},    \
                            concat(__VA_ARGS__))
#endif

#ifndef VENUS_INFO
#define VENUS_INFO(...)                                                        \
  venus::core::Log::message(venus::core::Log::Level::info,                     \
                            venus::core::Log::Location{__FILE__, __LINE__},    \
                            concat(__VA_ARGS__))
#endif

#ifndef VENUS_WARN
#define VENUS_WARN(...)                                                        \
  venus::core::Log::message(venus::core::Log::Level::warn,                     \
                            venus::core::Log::Location{__FILE__, __LINE__},    \
                            concat(__VA_ARGS__))
#endif

#ifndef VENUS_ERROR
#define VENUS_ERROR(...)                                                       \
  venus::core::Log::message(venus::core::Log::Level::error,                    \
                            venus::core::Log::Location{__FILE__, __LINE__},    \
                            concat(__VA_ARGS__))
#endif

#ifndef VENUS_CRITICAL
#define VENUS_CRITICAL(...)                                                    \
  venus::core::Log::message(venus::core::Log::Level::critical,                 \
                            venus::core::Log::Location{__FILE__, __LINE__},    \
                            concat(__VA_ARGS__))
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
    return "VK_TIMEOUT A wait operation has not completed in the specified "
           "time";
  case VK_EVENT_SET:
    return "VK_EVENT_SET An event is signaled";
  case VK_EVENT_RESET:
    return "VK_EVENT_RESET An event is unsignaled";
  case VK_INCOMPLETE:
    return "VK_INCOMPLETE A return array was too small for the result";
  case VK_SUBOPTIMAL_KHR:
    return "VK_SUBOPTIMAL_KHR A swapchain no longer matches the surface "
           "properties exactly, but can still be used to present to the "
           "surface successfully.";
  case VK_ERROR_OUT_OF_HOST_MEMORY:
    return "VK_ERROR_OUT_OF_HOST_MEMORY A host memory allocation has failed.";
  case VK_ERROR_OUT_OF_DEVICE_MEMORY:
    return "VK_ERROR_OUT_OF_DEVICE_MEMORY A device memory allocation has "
           "failed.";
  case VK_ERROR_INITIALIZATION_FAILED:
    return "VK_ERROR_INITIALIZATION_FAILED Initialization of an object could "
           "not be completed for implementation-specific reasons.";
  case VK_ERROR_DEVICE_LOST:
    return "VK_ERROR_DEVICE_LOST The logical or physical device has been "
           "lost. ";
  case VK_ERROR_MEMORY_MAP_FAILED:
    return "VK_ERROR_MEMORY_MAP_FAILED Mapping of a memory object has failed.";
  case VK_ERROR_LAYER_NOT_PRESENT:
    return "VK_ERROR_LAYER_NOT_PRESENT A requested layer is not present or "
           "could not be loaded.";
  case VK_ERROR_EXTENSION_NOT_PRESENT:
    return "VK_ERROR_EXTENSION_NOT_PRESENT A requested extension is not "
           "supported.";
  case VK_ERROR_FEATURE_NOT_PRESENT:
    return "VK_ERROR_FEATURE_NOT_PRESENT A requested feature is not supported.";
  case VK_ERROR_INCOMPATIBLE_DRIVER:
    return "VK_ERROR_INCOMPATIBLE_DRIVER The requested version of Vulkan is "
           "not supported by the driver or is otherwise incompatible for "
           "implementation-specific reasons.";
  case VK_ERROR_TOO_MANY_OBJECTS:
    return "VK_ERROR_TOO_MANY_OBJECTS Too many objects of the type have "
           "already been created.";
  case VK_ERROR_FORMAT_NOT_SUPPORTED:
    return "VK_ERROR_FORMAT_NOT_SUPPORTED A requested format is not supported "
           "on this device.";
  case VK_ERROR_FRAGMENTED_POOL:
    return "VK_ERROR_FRAGMENTED_POOL A pool allocation has failed due to "
           "fragmentation of the pool’s memory. This must only be returned if "
           "no attempt to allocate host or device memory was made to "
           "accommodate the new allocation. This should be returned in "
           "preference to VK_ERROR_OUT_OF_POOL_MEMORY, but only if the "
           "implementation is certain that the pool allocation failure was due "
           "to fragmentation.";
  case VK_ERROR_SURFACE_LOST_KHR:
    return "VK_ERROR_SURFACE_LOST_KHR A surface is no longer available.";
  case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
    return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR The requested window is already "
           "in use by Vulkan or another API in a manner which prevents it from "
           "being used again.";
  case VK_ERROR_OUT_OF_DATE_KHR:
    return "VK_ERROR_OUT_OF_DATE_KHR A surface has changed in such a way that "
           "it is no longer compatible with the swapchain, and further "
           "presentation requests using the swapchain will fail. Applications "
           "must query the new surface properties and recreate their swapchain "
           "if they wish to continue presenting to the surface.";
  case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
    return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR The display used by a swapchain "
           "does not use the same presentable image layout, or is incompatible "
           "in a way that prevents sharing an image.";
  case VK_ERROR_INVALID_SHADER_NV:
    return "VK_ERROR_INVALID_SHADER_NV One or more shaders failed to compile "
           "or link. More details are reported back to the application via "
           "https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/"
           "vkspec.html#VK_EXT_debug_report if enabled.";
#ifndef __linux
  case VK_ERROR_OUT_OF_POOL_MEMORY:
    return "VK_ERROR_OUT_OF_POOL_MEMORY A pool memory allocation has failed. "
           "This must only be returned if no attempt to allocate host or "
           "device memory was made to accommodate the new allocation. If the "
           "failure was definitely due to fragmentation of the pool, "
           "VK_ERROR_FRAGMENTED_POOL should be returned instead.";
  case VK_ERROR_INVALID_EXTERNAL_HANDLE:
    return "VK_ERROR_INVALID_EXTERNAL_HANDLE An external handle is not a valid "
           "handle of the specified type.";
  case VK_ERROR_FRAGMENTATION_EXT:
    return "VK_ERROR_FRAGMENTATION_EXT A descriptor pool creation has failed "
           "due to fragmentation.";
#ifndef WIN32
  case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
    return "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT A buffer creation failed "
           "because the requested address is not available.";
  case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
    return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT An operation on a "
           "swapchain created with "
           "VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT failed as it "
           "did not have exlusive full-screen access. This may occur due to "
           "implementation-dependent reasons, outside of the application’s "
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

#define CHECK_VULKAN(A)                                                        \
  {                                                                            \
    VkResult err = (A);                                                        \
    if (err != VK_SUCCESS) {                                                   \
      VENUS_ERROR("{}", #A);                                                   \
      VENUS_ERROR("{}", vulkanResultString(err));                              \
    }                                                                          \
  }
///
#define R_CHECK_VULKAN(A, R)                                                   \
  {                                                                            \
    VkResult err = (A);                                                        \
    if (err != VK_SUCCESS) {                                                   \
      VENUS_ERROR("{}", #A);                                                   \
      VENUS_ERROR("{}", vulkanResultString(err));                              \
      return R;                                                                \
    }                                                                          \
  }
///
#define ASSERT_VULKAN(A)                                                       \
  {                                                                            \
    VkResult err = (A);                                                        \
    if (err != VK_SUCCESS) {                                                   \
      VENUS_ERROR(#A);                                                         \
      VENUS_ERROR(vulkanResultString(err));                                    \
      exit(-1);                                                                \
    }                                                                          \
  }
