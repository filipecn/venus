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

/// \file   shader_module.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30

#include <venus/pipeline/shader_module.h>

#include <venus/utils/vk_debug.h>

#include <fstream>

#include <SPIRV/GlslangToSpv.h>
#include <SPIRV/spirv.hpp>

#ifdef __cpp_lib_byteswap
#include <bit>
// using std::byteswap
#elif defined(_MSC_VER)
#include <stdlib.h>
// using _byteswap_ulong
#endif

namespace venus::pipeline {

ShaderModule::Config &
ShaderModule::Config::setEntryFuncName(const std::string &name) {
  entry_function_name_ = name;
  return *this;
}

static inline std::uint32_t swapEndianness(std::uint32_t word) {
#ifdef __cpp_lib_byteswap /* C++23 available */
  return std::byteswap(word);
#elif defined(__GNUC__)
  return __builtin_bswap32(word);
#elif defined(_MSC_VER)
  return _byteswap_ulong(word);
#else
  /*
   * insert rant about the late addition of <bit> header
   * and remaining lack of hton / ntoh equivalents here
   */
  return ((word & 0xff) << 24) | ((word & 0xff00) << 8) |
         ((word & 0xff0000) >> 8) | ((word & 0xff000000) >> 24);
#endif
}

Result<std::vector<std::uint32_t>> readFile(const std::string &filename) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);
  if (!file.is_open()) {
    HERMES_ERROR("failed to open file");
    return VeResult::ioError();
  }
  auto fileSize = file.tellg();
  if (!fileSize) { // explicit check required as we access rtrn.front() later
    HERMES_ERROR("file appears empty");
    return VeResult::ioError();
  }
  if (fileSize % sizeof(std::uint32_t)) {
    HERMES_ERROR("file size not a multiple of word size");
    return VeResult::ioError();
  }
  file.seekg(0);
  std::vector<std::uint32_t> rtrn(fileSize / sizeof(std::uint32_t));
  if (!file.read(reinterpret_cast<char *>(rtrn.data()), fileSize)) {
    HERMES_ERROR("file reading failed");
    return VeResult::ioError();
  }
  if (rtrn.front() == swapEndianness(spv::MagicNumber))
    for (auto &word : rtrn)
      word = swapEndianness(word);
  else if (rtrn.front() != spv::MagicNumber) {
    HERMES_ERROR("unrecognized file format");
    return VeResult::ioError();
  }
  return Result<std::vector<std::uint32_t>>(std::move(rtrn));
}

ShaderModule::Config &
ShaderModule::Config::fromSpvFile(const std::filesystem::path &path_to_spv) {
  VENUS_ASSIGN(spirv_, readFile(path_to_spv));
  return *this;
}

ShaderModule::ShaderModule(ShaderModule &&rhs) noexcept {
  *this = std::move(rhs);
}

ShaderModule &ShaderModule::operator=(ShaderModule &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

ShaderModule::~ShaderModule() noexcept { destroy(); }

void ShaderModule::swap(ShaderModule &rhs) noexcept {
  VENUS_SWAP_FIELD_WITH_RHS(vk_device_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_shader_module_);
  VENUS_SWAP_FIELD_WITH_RHS(name_);
}

void ShaderModule::destroy() noexcept {
  if (vk_device_ && vk_shader_module_)
    vkDestroyShaderModule(vk_device_, vk_shader_module_, nullptr);
  vk_shader_module_ = VK_NULL_HANDLE;
  vk_device_ = VK_NULL_HANDLE;
}

VkShaderModule ShaderModule::operator*() const { return vk_shader_module_; }

const std::string &ShaderModule::name() const { return name_; }

Result<ShaderModule>
ShaderModule::Config::build(VkDevice vk_device,
                            VkShaderModuleCreateFlags flags) const {
  ShaderModule module;

  VkShaderModuleCreateInfo info;
  info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  info.pNext = nullptr;
  info.codeSize = sizeof(uint32_t) * spirv_.size();
  info.pCode = spirv_.data();
  info.flags = flags;

  VENUS_VK_RETURN_BAD_RESULT(vkCreateShaderModule(vk_device, &info, nullptr,
                                                  &module.vk_shader_module_));

  module.vk_device_ = vk_device;
  module.name_ = entry_function_name_;
  return Result<ShaderModule>(std::move(module));
}

} // namespace venus::pipeline

namespace venus {

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::pipeline::ShaderModule)
HERMES_PUSH_DEBUG_VK_HANDLE(vk_shader_module_)
HERMES_PUSH_DEBUG_VK_HANDLE(vk_device_)
HERMES_PUSH_DEBUG_FIELD(name_)
HERMES_TO_STRING_DEBUG_METHOD_END

} // namespace venus
