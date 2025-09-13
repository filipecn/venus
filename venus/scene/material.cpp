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

Material::Instance::Instance(Material::Instance &&rhs) noexcept {
  *this = std::move(rhs);
}

Material::Instance &
Material::Instance::operator=(Material::Instance &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

Material::Instance::~Instance() noexcept { destroy(); }

void Material::Instance::swap(Material::Instance &rhs) {
  VENUS_SWAP_FIELD_WITH_RHS(material);
  VENUS_SWAP_FIELD_WITH_RHS(descriptor_set);
}

void Material::Instance::destroy() noexcept {
  material = nullptr;
  descriptor_set.destroy();
}

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(Material::Pipeline, setPipelineConfig,
                                     const pipeline::GraphicsPipeline::Config &,
                                     pipeline_config_ = value)
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(Material::Pipeline,
                                     setPipelineLayoutConfig,
                                     const pipeline::Pipeline::Layout::Config &,
                                     layout_config_ = value)

Result<Material::Pipeline>
Material::Pipeline::Config::create(VkDevice vk_device,
                                   VkRenderPass vk_renderpass) const {
  Material::Pipeline p;

  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(p.pipeline_layout_,
                                           layout_config_.create(vk_device));

  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      p.pipeline_,
      pipeline_config_.create(vk_device, *p.pipeline_layout_, vk_renderpass));

#ifdef VENUS_DEBUG
  p.config_ = *this;
#endif

  return Result<Material::Pipeline>(std::move(p));
}

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(Material, setMaterialPipelineConfig,
                                     const Material::Pipeline::Config &,
                                     pipeline_config_ = value)
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(Material, setDescriptorSetLayout,
                                     pipeline::DescriptorSet::Layout &&,
                                     descriptor_set_layout_ = std::move(value))

Result<Material> Material::Config::create(VkDevice vk_device,
                                          VkRenderPass vk_renderpass) const {

  Material material;

  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      material.pipeline_, pipeline_config_.create(vk_device, vk_renderpass));

  return Result<Material>(std::move(material));
}

Material::Pipeline::Pipeline(Material::Pipeline &&rhs) noexcept {
  *this = std::move(rhs);
}

Material::Pipeline::~Pipeline() noexcept { destroy(); }

Material::Pipeline &
Material::Pipeline::operator=(Material::Pipeline &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void Material::Pipeline::swap(Material::Pipeline &rhs) {
  VENUS_SWAP_FIELD_WITH_RHS(pipeline_);
  VENUS_SWAP_FIELD_WITH_RHS(pipeline_layout_);
}

void Material::Pipeline::destroy() noexcept {
  pipeline_.destroy();
  pipeline_layout_.destroy();
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
  VENUS_SWAP_FIELD_WITH_RHS(descriptor_set_layout_);
}

void Material::destroy() noexcept {
  pipeline_.destroy();
  descriptor_set_layout_.destroy();
}

const pipeline::DescriptorSet::Layout &Material::descriptorSetLayout() const {
  return descriptor_set_layout_;
}

} // namespace venus::scene

namespace venus {

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::scene::Material::Pipeline::Config)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_VENUS_FIELD(pipeline_config_)
HERMES_PUSH_DEBUG_VENUS_FIELD(layout_config_)
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::scene::Material::Pipeline)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_VENUS_FIELD(pipeline_)
HERMES_PUSH_DEBUG_VENUS_FIELD(pipeline_layout_)
HERMES_PUSH_DEBUG_VENUS_FIELD(config_)
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::scene::Material::Config)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_VENUS_FIELD(descriptor_set_layout_)
HERMES_PUSH_DEBUG_VENUS_FIELD(pipeline_config_)
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::scene::Material)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_VENUS_FIELD(pipeline_)
HERMES_PUSH_DEBUG_VENUS_FIELD(descriptor_set_layout_);
HERMES_PUSH_DEBUG_VENUS_FIELD(config_)
HERMES_TO_STRING_DEBUG_METHOD_END

} // namespace venus
