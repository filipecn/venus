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

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(Material::Instance, setMaterial,
                                     Material::Ptr, material_ = value)

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(Material::Instance, addGlobalSetIndex,
                                     h_index, global_set_indices_.insert(value))

Material::Instance::Config &Material::Instance::Config::setWritePushConstants(
    VkShaderStageFlags stage_flags,
    const std::function<VeResult(hermes::mem::Block &,
                                 const PushConstantsContext &)> &f) {
  push_constants_stage_flags_ = stage_flags;
  write_push_constants_ = f;
  return *this;
}

Result<Material::Instance> Material::Instance::Config::build(
    pipeline::DescriptorAllocator &allocator) const {
  Material::Instance instance;

  instance.material_ = material_;
  instance.write_push_constants_ = write_push_constants_;
  instance.push_constants_stage_flags_ = push_constants_stage_flags_;
  instance.global_set_indices_ = global_set_indices_;

  // allocate local descriptor sets
  const auto &all_descriptor_set_layouts = material_->descriptorSetLayouts();
  for (const auto &item : all_descriptor_set_layouts) {
    if (global_set_indices_.count(item.first) > 0)
      continue;
    VENUS_DECLARE_OR_RETURN_BAD_RESULT(pipeline::DescriptorSet, descriptor_set,
                                       allocator.allocate(item.second));
    instance.descriptor_sets_[item.first] = std::move(descriptor_set);
  }

  return Result<Material::Instance>(std::move(instance));
}

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
  VENUS_SWAP_FIELD_WITH_RHS(material_);
  VENUS_SWAP_FIELD_WITH_RHS(descriptor_sets_);
  VENUS_SWAP_FIELD_WITH_RHS(global_set_indices_);
  VENUS_SWAP_FIELD_WITH_RHS(write_push_constants_);
  VENUS_SWAP_FIELD_WITH_RHS(push_constants_stage_flags_);
}

void Material::Instance::destroy() noexcept {
  material_.destroy();
  for (auto &ds : descriptor_sets_)
    ds.second.destroy();
  write_push_constants_ = nullptr;
}

Material::Ptr Material::Instance::material() const { return material_; }

const pipeline::GraphicsPipeline &Material::Instance::pipeline() const {
  return material_->pipeline().pipeline();
}

const pipeline::Pipeline::Layout &Material::Instance::pipelineLayout() const {
  return material_->pipeline().pipelineLayout();
}

bool Material::Instance::hasGlobalDescriptors() const {
  return global_set_indices_.size();
}

std::unordered_map<h_index, std::vector<VkDescriptorSet>>
Material::Instance::localDescriptorSetGroups() const {
  if (descriptor_sets_.empty())
    return {};
  // get all set indices in increasing order so we can create the contiguous
  // groups
  std::set<h_index> set_indices;
  for (const auto &item : descriptor_sets_)
    set_indices.insert(item.first);

  std::unordered_map<h_index, std::vector<VkDescriptorSet>> groups;
  h_index last_set_index = 0;
  for (auto set_index : set_indices) {
    auto it = descriptor_sets_.find(set_index);
    if (groups.empty()) {
      groups[set_index] = {*(it->second)};
    } else if (last_set_index + 1 == set_index) {
      groups[set_index].emplace_back(*(it->second));
    } else {
      groups[set_index] = {*(it->second)};
    }
    last_set_index = set_index;
  }
  return groups;
}

const pipeline::DescriptorSet &
Material::Instance::localDescriptorSet(h_index set_index) const {
  static pipeline::DescriptorSet s_dummy;
  auto it = descriptor_sets_.find(set_index);
  if (it == descriptor_sets_.end()) {
    HERMES_ERROR(
        "Descriptor set with set index {} not found in material instance.",
        set_index);
    return s_dummy;
  }
  return it->second;
}

VeResult
Material::Instance::writePushConstants(hermes::mem::Block &block,
                                       const PushConstantsContext &ctx) const {
  if (write_push_constants_)
    return write_push_constants_(block, ctx);
  VENUS_RETURN_BAD_HE_RESULT(block.clear());
  return VeResult::noError();
}

VkShaderStageFlags Material::Instance::pushConstantsStageFlags() const {
  return push_constants_stage_flags_;
}

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(Material::Pipeline, setPipelineConfig,
                                     const pipeline::GraphicsPipeline::Config &,
                                     graphics_pipeline_config_ = value)
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(Material::Pipeline,
                                     setPipelineLayoutConfig,
                                     const pipeline::Pipeline::Layout::Config &,
                                     graphics_pipeline_layout_config_ = value)

const pipeline::Pipeline::Layout::Config &
Material::Pipeline::Config::graphicsPipelineLayoutConfig() const {
  return graphics_pipeline_layout_config_;
}

Result<Material::Pipeline>
Material::Pipeline::Config::build(VkDevice vk_device,
                                  VkRenderPass vk_renderpass) const {
  Material::Pipeline p;

  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      p.pipeline_layout_, graphics_pipeline_layout_config_.build(vk_device));

  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      p.pipeline_, graphics_pipeline_config_.build(
                       vk_device, *p.pipeline_layout_, vk_renderpass));

#ifdef VENUS_DEBUG
  p.config_ = *this;
#endif

  return Result<Material::Pipeline>(std::move(p));
}

Result<Material::Instance>
Material::Writer::write(pipeline::DescriptorAllocator &allocator,
                        Material::Ptr material) {
  Material::Instance instance;

  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      instance,
      Material::Instance::Config().setMaterial(material).build(allocator))

  return Result<Material::Instance>(std::move(instance));
}

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(Material, setMaterialPipelineConfig,
                                     const Material::Pipeline::Config &,
                                     pipeline_config_ = value)

Result<Material> Material::Config::build(VkDevice vk_device,
                                         VkRenderPass vk_renderpass) const {

  Material material;

  // get list of descriptor sets
  const auto &descriptor_set_layouts =
      pipeline_config_.graphicsPipelineLayoutConfig().descriptorSetLayouts();
  for (h_index i = 0; i < descriptor_set_layouts.size(); ++i)
    material.vk_descriptor_set_layouts_[i] = descriptor_set_layouts[i];

  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      material.pipeline_, pipeline_config_.build(vk_device, vk_renderpass));

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

const pipeline::GraphicsPipeline &Material::Pipeline::operator*() const {
  return pipeline_;
}

const pipeline::GraphicsPipeline &Material::Pipeline::pipeline() const {
  return pipeline_;
}

VkPipeline Material::Pipeline::vkPipeline() const { return *pipeline_; }

const pipeline::Pipeline::Layout &Material::Pipeline::pipelineLayout() const {
  return pipeline_layout_;
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
  VENUS_SWAP_FIELD_WITH_RHS(owned_descriptor_set_layouts_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_descriptor_set_layouts_);
}

void Material::destroy() noexcept {
  pipeline_.destroy();
  for (auto &dsl : owned_descriptor_set_layouts_)
    dsl.destroy();
  owned_descriptor_set_layouts_.clear();
  vk_descriptor_set_layouts_.clear();
}

const Material::Pipeline &Material::pipeline() const { return pipeline_; }

void Material::ownDescriptorSetLayout(
    pipeline::DescriptorSet::Layout &&descriptor_set_layout) {
  owned_descriptor_set_layouts_.emplace_back(std::move(descriptor_set_layout));
}

const std::unordered_map<h_index, VkDescriptorSetLayout> &
Material::descriptorSetLayouts() const {
  return vk_descriptor_set_layouts_;
}

} // namespace venus::scene
