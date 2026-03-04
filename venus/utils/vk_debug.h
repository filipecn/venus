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

/// \file   debug.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-06-07
/// \brief  Auxiliary functions for debugging and error handling.

#pragma once

#include <hermes/core/debug.h>
#include <venus/core/vk_api.h>

#include <vulkan/vk_enum_string_helper.h>

#ifdef VENUS_INCLUDE_DEBUG_TRAITS

#ifndef VENUS_VK_STRING
#define VENUS_VK_STRING(T, O) string_##T(O)
#endif

// examples of dispatchable objects:
// VkInstance
// VkPhysicalDevice
// VkDevice
// VkQueue
// VkCommandBuffer
#ifndef VENUS_VK_DISPATCHABLE_HANDLE_STRING
#define VENUS_VK_DISPATCHABLE_HANDLE_STRING(H)                                 \
  (H == VK_NULL_HANDLE                                                         \
       ? "VK_NULL_HANDLE"                                                      \
       : hermes::cstr::format("{:p}", static_cast<void *>(H)).c_str())
#endif

#ifndef VENUS_VK_HANDLE_STRING
#define VENUS_VK_HANDLE_STRING(H)                                              \
  (H == VK_NULL_HANDLE                                                         \
       ? "VK_NULL_HANDLE"                                                      \
       : hermes::cstr::format("0x{:016X}", reinterpret_cast<uint64_t>(H))      \
             .c_str())
#endif

#else
#define VENUS_VK_STRING
#endif

namespace venus::core {

/// Retrieves the description of VkResult values
/// \param[in] err error code
/// \return std::string error description
inline std::string_view vulkanResultString(VkResult err) {
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
    return string_VkResult(err);
  }
  return "UNDEFINED";
}

} // namespace venus::core

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
namespace hermes {

template <> struct DebugTraits<VkFormat> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage message(const VkFormat &data) {
    return DebugMessage("{}", string_VkFormat(data));
  }
};

template <> struct DebugTraits<VkPipelineShaderStageCreateInfo> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage message(const VkPipelineShaderStageCreateInfo &data) {
    return DebugMessage("stage {} module 0x{:x}\n",
                        string_VkShaderStageFlagBits(data.stage),
                        (uintptr_t)(data.module));
  }
};

template <> struct DebugTraits<VkExtent3D> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage message(const VkExtent3D &data) {
    return DebugMessage("[{}x{}x{}]", data.width, data.height, data.depth);
  }
};

template <> struct DebugTraits<VkExtent2D> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage message(const VkExtent2D &data) {
    return DebugMessage("[{}x{}]", data.width, data.height);
  }
};

template <> struct DebugTraits<VkOffset3D> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage message(const VkOffset3D &data) {
    return DebugMessage("[{}x{}x{}]", data.x, data.y, data.z);
  }
};

template <> struct DebugTraits<VkOffset2D> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage message(const VkOffset2D &data) {
    return DebugMessage("[{}x{}]", data.x, data.y);
  }
};

template <> struct DebugTraits<VkSpecializationMapEntry> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage message(const VkSpecializationMapEntry &data) {
    return DebugMessage("constantID {} offset {} size {}", data.constantID,
                        data.offset, data.size);
  }
};

template <> struct DebugTraits<VkVertexInputBindingDescription> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage message(const VkVertexInputBindingDescription &data) {
    return DebugMessage("binding {} stride {} inputRage {}", data.binding,
                        data.stride, string_VkVertexInputRate(data.inputRate));
  }
};

template <> struct DebugTraits<VkVertexInputAttributeDescription> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage message(const VkVertexInputAttributeDescription &data) {
    return DebugMessage("location {} binding {} format {} offset {}",
                        data.location, data.binding,
                        string_VkFormat(data.format), data.offset);
  }
};

template <> struct DebugTraits<VkViewport> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage message(const VkViewport &data) {
    return DebugMessage("{} {} {} {} {} {}\n", data.x, data.y, data.width,
                        data.height, data.minDepth, data.maxDepth);
  }
};

template <> struct DebugTraits<VkRect2D> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage message(const VkRect2D &data) {
    return DebugMessage()
        .addTitle("VkRect2D")
        .add("offset", data.offset)
        .add("extent", data.extent);
  }
};

template <> struct DebugTraits<VkStencilOpState> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage message(const VkStencilOpState &data) {
    return DebugMessage("info_.front = failOp {} passOp {} compareOp {} "
                        "compareMask {} writeMask {} reference {}",
                        string_VkStencilOp(data.failOp),
                        string_VkStencilOp(data.passOp),
                        string_VkStencilOp(data.depthFailOp),
                        string_VkCompareOp(data.compareOp), data.compareMask,
                        data.writeMask, data.reference);
  }
};

template <> struct DebugTraits<VkPushConstantRange> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage message(const VkPushConstantRange &data) {
    return DebugMessage("stageFlags: {} offset {} size {}",
                        string_VkShaderStageFlags(data.stageFlags), data.offset,
                        data.size);
  }
};

template <> struct DebugTraits<VkPipelineColorBlendAttachmentState> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage message(const VkPipelineColorBlendAttachmentState &data) {
    return DebugMessage()
        .addTitle("Blend Attachment State")
        .add("blendEnable", data.blendEnable)
        .add("srcColorBlendFactor",
             string_VkBlendFactor(data.srcColorBlendFactor))
        .add("dstColorBlendFactor",
             string_VkBlendFactor(data.dstColorBlendFactor))
        .add("colorBlendOp", string_VkBlendOp(data.colorBlendOp))
        .add("srcAlphaBlendFactor",
             string_VkBlendFactor(data.srcAlphaBlendFactor))
        .add("dstAlphaBlendFactor",
             string_VkBlendFactor(data.dstAlphaBlendFactor))
        .add("alphaBlendOp", string_VkBlendOp(data.alphaBlendOp))
        .add("colorWriteMask",
             string_VkColorComponentFlags(data.colorWriteMask));
  }
};

template <> struct DebugTraits<VkMemoryRequirements> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage message(const VkMemoryRequirements &data) {
    return DebugMessage()
        .addTitle("Memory Requirements")
        .add("size", data.size)
        .add("alignment", data.alignment)
        .add("memory type bits", data.memoryTypeBits);
  }
};

template <> struct DebugTraits<VkDescriptorSetLayout> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage message(const VkDescriptorSetLayout &data) {
    return DebugMessage("{}", VENUS_VK_HANDLE_STRING(data));
  }
};

template <> struct DebugTraits<VkDescriptorSetLayoutBinding> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage message(const VkDescriptorSetLayoutBinding &data) {
    return DebugMessage("{} {} {} {}", data.binding,
                        VENUS_VK_STRING(VkDescriptorType, data.descriptorType),
                        data.descriptorCount,
                        VENUS_VK_STRING(VkShaderStageFlags, data.stageFlags));
  }
};

template <> struct DebugTraits<VkImageCreateInfo> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage message(const VkImageCreateInfo &data) {
    return DebugMessage()
        .addTitle("VkImageCreateInfo")
        .add("flags", VENUS_VK_STRING(VkImageCreateFlags, data.flags))
        .add("imageType", VENUS_VK_STRING(VkImageType, data.imageType))
        .add("format", VENUS_VK_STRING(VkFormat, data.format))
        .add("extent", data.extent)
        .add("mipLevels", data.mipLevels)
        .add("arrayLayers", data.arrayLayers)
        .add("samples", VENUS_VK_STRING(VkSampleCountFlagBits, data.samples))
        .add("tiling", VENUS_VK_STRING(VkImageTiling, data.tiling))
        .add("usage", VENUS_VK_STRING(VkImageUsageFlags, data.usage))
        .add("sharingMode", VENUS_VK_STRING(VkSharingMode, data.sharingMode))
        .add("queueFamilyIndexCount", data.queueFamilyIndexCount)
        //.add("pQueueFamilyInd",
        //     VENUS_VK_STRING(const uint32_t *, data.pQueueFamilyIndices))
        .add("initialLayout",
             VENUS_VK_STRING(VkImageLayout, data.initialLayout));
  }
};

} // namespace hermes
#endif // VENUS_INCLUDE_DEBUG_TRAITS

#define VENUS_VK_RETURN_BAD_RESULT(A)                                          \
  {                                                                            \
    VkResult _venus_vk_return_ve_error_ = (A);                                 \
    if (_venus_vk_return_ve_error_ != VK_SUCCESS) {                            \
      HERMES_ERROR("vulkan call error: {}", #A);                               \
      HERMES_ERROR(                                                            \
          "{}", venus::core::vulkanResultString(_venus_vk_return_ve_error_));  \
      return VeResult::vkError();                                              \
    }                                                                          \
  }
#define VENUS_VK_RETURN_BAD(A, R)                                              \
  {                                                                            \
    VkResult _venus_vk_return_bad_ = (A);                                      \
    if (_venus_vk_return_bad_ != VK_SUCCESS) {                                 \
      HERMES_ERROR("vulkan call error: {}", #A);                               \
      HERMES_ERROR("{}",                                                       \
                   venus::core::vulkanResultString(_venus_vk_return_bad_));    \
      return R;                                                                \
    }                                                                          \
  }
#define VENUS_VK_ASSERT(A)                                                     \
  {                                                                            \
    VkResult _venus_vk_assert_ = (A);                                          \
    if (_venus_vk_assert_ != VK_SUCCESS) {                                     \
      HERMES_ERROR(#A);                                                        \
      HERMES_ERROR(venus::core::vulkanResultString(_venus_vk_assert_));        \
      exit(-1);                                                                \
    }                                                                          \
  }
