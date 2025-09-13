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

/// \file   shader_module.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30
/// \brief  Vulkan shader module

#pragma once

#include <venus/core/device.h>

namespace venus::pipeline {

/// The computations executed inside the pipeline are performed by shaders.
/// Shaders are represented by Shader Modules and must be provided to Vulkan as
/// SPIR-V assembly code. A single module may contain code for multiple shader
/// stages.
/// \note This class uses RAII.
class ShaderModule {
public:
  /// Builder for shader module class.
  struct Config {
    /// \note  this loads a new shader module and discards any previous loaded
    ///        module.
    /// \note  this expects a binary file.
    /// \note  this infers the stage from the file, no need to call setStage if
    ///        this method is used.
    /// \param path_to_spv Path to file containing shader code.
    Config &fromSpvFile(const std::filesystem::path &path_to_spv);
    /// \note this is optional, as "main" is the default internal value for this
    ///       field.
    /// \param name Entry function name.
    Config &setEntryFuncName(const std::string &name);

    Result<ShaderModule> create(VkDevice vk_device,
                                VkShaderModuleCreateFlags flags = {}) const;

  private:
    std::string entry_function_name_{"main"};
    std::vector<std::uint32_t> spirv_;
  };

  // RAII

  VENUS_DECLARE_RAII_FUNCTIONS(ShaderModule);

  void destroy() noexcept;
  void swap(ShaderModule &rhs) noexcept;
  VkShaderModule operator*() const;
  const std::string &name() const;

private:
  VkShaderModule vk_shader_module_{VK_NULL_HANDLE};
  VkDevice vk_device_{VK_NULL_HANDLE};
  std::string name_;

  VENUS_TO_STRING_FRIEND(ShaderModule);
};

} // namespace venus::pipeline
