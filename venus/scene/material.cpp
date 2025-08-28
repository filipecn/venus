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

/// \file   material.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30

#include <venus/scene/material.h>

namespace venus::scene {

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(Material, setPipelineConfig,
                                     const pipeline::GraphicsPipeline::Config &,
                                     pipeline_config_ = value)
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(Material, setPipelineLayoutConfig,
                                     const pipeline::Pipeline::Layout::Config &,
                                     layout_config_ = value)

Result<Material> Material::Config::create(VkDevice vk_device,
                                          VkRenderPass vk_renderpass) const {
  Material material;

  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(material.pipeline_layout_,
                                           layout_config_.create(vk_device));

  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      material.pipeline_,
      pipeline_config_.create(vk_device, *material.pipeline_layout_,
                              vk_renderpass));

  return Result<Material>(std::move(material));
}

Material::Material(Material &&rhs) noexcept { *this = std::move(rhs); }

Material::~Material() noexcept { destroy(); }

Material &Material::operator=(Material &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void Material::swap(Material &rhs) {
  VENUS_SWAP_FIELD_WITH_RHS(pipeline_);
  VENUS_SWAP_FIELD_WITH_RHS(pipeline_layout_);
  VENUS_SWAP_FIELD_WITH_RHS(descriptor_writer_);
}

void Material::destroy() noexcept {
  pipeline_.destroy();
  pipeline_layout_.destroy();
  descriptor_writer_.clear();
}

} // namespace venus::scene
