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

/// \file   pipeline.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30
/// \brief  Vulkan pipeline

#include <venus/pipeline/pipeline.h>

#include <venus/utils/vk_debug.h>

namespace venus {

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::pipeline::Pipeline::ShaderStage)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_VK_STRING(VkShaderStageFlagBits, stages_);
HERMES_PUSH_DEBUG_ARRAY_FIELD_BEGIN(specialization_map_entries_, entry)
HERMES_PUSH_DEBUG_LINE("constantID {} offset {} size {}", entry.constantID,
                       entry.offset, entry.size)
HERMES_PUSH_DEBUG_ARRAY_FIELD_END
HERMES_PUSH_DEBUG_FIELD(specialization_data_size_)
HERMES_PUSH_DEBUG_FIELD(specialization_data_)
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::pipeline::Pipeline::Layout::Config)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_VK_STRING(VkPipelineLayoutCreateFlags, flags_);
HERMES_PUSH_DEBUG_ARRAY_FIELD_BEGIN(ranges_, range)
HERMES_PUSH_DEBUG_LINE("stageFlags: {} offset {} size {}",
                       string_VkShaderStageFlags(range.stageFlags),
                       range.offset, range.size)
HERMES_PUSH_DEBUG_ARRAY_FIELD_END
HERMES_PUSH_DEBUG_ARRAY_FIELD_BEGIN(set_layouts_, set_layout)
HERMES_PUSH_DEBUG_LINE("0x{:x}", (uintptr_t)set_layout);
HERMES_PUSH_DEBUG_ARRAY_FIELD_END
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::pipeline::Pipeline::Layout)
HERMES_PUSH_DEBUG_VK_FIELD(vk_layout_);
HERMES_PUSH_DEBUG_VK_FIELD(vk_device_);
HERMES_PUSH_DEBUG_VENUS_FIELD(config_);
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::pipeline::Pipeline)
HERMES_PUSH_DEBUG_VK_FIELD(vk_pipeline_);
HERMES_PUSH_DEBUG_VK_FIELD(vk_pipeline_cache_);
HERMES_PUSH_DEBUG_VK_FIELD(vk_device_);
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(
    venus::pipeline::GraphicsPipeline::VertexInput)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_ARRAY_FIELD_BEGIN(binding_descriptions_, d)
HERMES_PUSH_DEBUG_LINE("binding {} stride {} inputRage {}", d.binding, d.stride,
                       string_VkVertexInputRate(d.inputRate))
HERMES_PUSH_DEBUG_ARRAY_FIELD_END
HERMES_PUSH_DEBUG_ARRAY_FIELD_BEGIN(attribute_descriptions_, a)
HERMES_PUSH_DEBUG_LINE("location {} binding {} format {} offset {}", a.location,
                       a.binding, string_VkFormat(a.format), a.offset)
HERMES_PUSH_DEBUG_ARRAY_FIELD_END
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(
    venus::pipeline::GraphicsPipeline::Rasterizer)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_FIELD(info_.depthClampEnable)
HERMES_PUSH_DEBUG_FIELD(info_.rasterizerDiscardEnable)
HERMES_PUSH_DEBUG_FIELD(info_.depthBiasEnable)
HERMES_PUSH_DEBUG_FIELD(info_.depthBiasConstantFactor)
HERMES_PUSH_DEBUG_FIELD(info_.depthBiasClamp)
HERMES_PUSH_DEBUG_FIELD(info_.depthBiasSlopeFactor)
HERMES_PUSH_DEBUG_FIELD(info_.lineWidth)
HERMES_PUSH_DEBUG_VK_STRING(VkPolygonMode, info_.polygonMode)
HERMES_PUSH_DEBUG_VK_STRING(VkCullModeFlags, info_.cullMode)
HERMES_PUSH_DEBUG_VK_STRING(VkFrontFace, info_.frontFace)
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(
    venus::pipeline::GraphicsPipeline::Multisample)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_VK_STRING(VkSampleCountFlagBits, info_.rasterizationSamples)
HERMES_PUSH_DEBUG_FIELD(info_.sampleShadingEnable)
HERMES_PUSH_DEBUG_FIELD(info_.minSampleShading)
HERMES_PUSH_DEBUG_FIELD(info_.pSampleMask)
HERMES_PUSH_DEBUG_FIELD(info_.alphaToCoverageEnable)
HERMES_PUSH_DEBUG_FIELD(info_.alphaToOneEnable)
HERMES_PUSH_DEBUG_ARRAY_FIELD_BEGIN(sample_masks_, s)
HERMES_PUSH_DEBUG_LINE("{}", (uintptr_t)s);
HERMES_PUSH_DEBUG_ARRAY_FIELD_END
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(
    venus::pipeline::GraphicsPipeline::ColorBlend)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_FIELD(info_.logicOpEnable)
HERMES_PUSH_DEBUG_VK_STRING(VkLogicOp, info_.logicOp)
HERMES_PUSH_DEBUG_FIELD(info_.attachmentCount)
HERMES_PUSH_DEBUG_ARRAY_FIELD_BEGIN(color_blend_attachments_, a)
HERMES_PUSH_DEBUG_LINE("[{}].blendEnable {}", i, a.blendEnable);
HERMES_PUSH_DEBUG_LINE("[{}].srcColorBlendFactor {}", i,
                       string_VkBlendFactor(a.srcColorBlendFactor))
HERMES_PUSH_DEBUG_LINE("[{}].dstColorBlendFactor {}", i,
                       string_VkBlendFactor(a.dstColorBlendFactor))
HERMES_PUSH_DEBUG_LINE("[{}].colorBlendOp)       {}", i,
                       string_VkBlendOp(a.colorBlendOp))
HERMES_PUSH_DEBUG_LINE("[{}].srcAlphaBlendFactor {}", i,
                       string_VkBlendFactor(a.srcAlphaBlendFactor))
HERMES_PUSH_DEBUG_LINE("[{}].dstAlphaBlendFactor {}", i,
                       string_VkBlendFactor(a.dstAlphaBlendFactor))
HERMES_PUSH_DEBUG_LINE("[{}].alphaBlendOp)       {}", i,
                       string_VkBlendOp(a.alphaBlendOp))
HERMES_PUSH_DEBUG_LINE("[{}].colorWriteMask)     {}", i,
                       string_VkColorComponentFlags(a.colorWriteMask))
HERMES_PUSH_DEBUG_ARRAY_FIELD_END
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(
    venus::pipeline::GraphicsPipeline::DepthStencil)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_FIELD(info_.depthTestEnable)
HERMES_PUSH_DEBUG_FIELD(info_.depthWriteEnable)
HERMES_PUSH_DEBUG_VK_STRING(VkCompareOp, info_.depthCompareOp)
HERMES_PUSH_DEBUG_FIELD(info_.depthBoundsTestEnable)
HERMES_PUSH_DEBUG_FIELD(info_.stencilTestEnable)
HERMES_PUSH_DEBUG_LINE("info_.front = failOp {} passOp {} compareOp {} "
                       "compareMask {} writeMask {} reference {}",
                       string_VkStencilOp(object.info_.front.failOp),
                       string_VkStencilOp(object.info_.front.passOp),
                       string_VkStencilOp(object.info_.front.depthFailOp),
                       string_VkCompareOp(object.info_.front.compareOp),
                       object.info_.front.compareMask,
                       object.info_.front.writeMask,
                       object.info_.front.reference)
HERMES_PUSH_DEBUG_LINE("info_.back = failOp {} passOp {} compareOp {} "
                       "compareMask {} writeMask {} reference {}",
                       string_VkStencilOp(object.info_.back.failOp),
                       string_VkStencilOp(object.info_.back.passOp),
                       string_VkStencilOp(object.info_.back.depthFailOp),
                       string_VkCompareOp(object.info_.back.compareOp),
                       object.info_.back.compareMask,
                       object.info_.back.writeMask, object.info_.back.reference)
HERMES_PUSH_DEBUG_FIELD(info_.minDepthBounds)
HERMES_PUSH_DEBUG_FIELD(info_.maxDepthBounds)
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::pipeline::GraphicsPipeline::Config)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_VENUS_FIELD(vertex_input_)
HERMES_PUSH_DEBUG_VENUS_FIELD(rasterization_)
HERMES_PUSH_DEBUG_VENUS_FIELD(color_blend_)
HERMES_PUSH_DEBUG_VENUS_FIELD(multisample_)
HERMES_PUSH_DEBUG_VENUS_FIELD(depth_stencil_)
HERMES_PUSH_DEBUG_LINE(
    "input_assembly = topology {} restartEnable {}",
    string_VkPrimitiveTopology(object.input_assembly_.topology),
    object.input_assembly_.primitiveRestartEnable)
HERMES_PUSH_DEBUG_ARRAY_FIELD_BEGIN(viewports_, v)
HERMES_PUSH_DEBUG_LINE("{} {} {} {} {} {}", v.x, v.y, v.width, v.height,
                       v.minDepth, v.maxDepth)
HERMES_PUSH_DEBUG_ARRAY_FIELD_END
HERMES_PUSH_DEBUG_ARRAY_FIELD_BEGIN(scissors_, s)
HERMES_PUSH_DEBUG_LINE("{} {} {} {}", s.offset.x, s.offset.y, s.extent.width,
                       s.extent.height)
HERMES_PUSH_DEBUG_ARRAY_FIELD_END
HERMES_PUSH_DEBUG_LINE("viewport_ = viewports {} scissors {}",
                       object.viewport_.viewportCount,
                       object.viewport_.scissorCount)
HERMES_PUSH_DEBUG_ARRAY_FIELD_BEGIN(color_attachment_formats_, f)
HERMES_PUSH_DEBUG_LINE("{}", string_VkFormat(f))
HERMES_PUSH_DEBUG_ARRAY_FIELD_END
HERMES_PUSH_DEBUG_ARRAY_FIELD_BEGIN(stages_, s)
HERMES_PUSH_DEBUG_LINE("stage {} module 0x{:x}",
                       string_VkShaderStageFlagBits(s.stage),
                       (uintptr_t)(s.module));
HERMES_PUSH_DEBUG_ARRAY_FIELD_END
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::pipeline::GraphicsPipeline)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_VENUS_FIELD(config_)
HERMES_TO_STRING_DEBUG_METHOD_END

} // namespace venus

namespace venus::pipeline {

Pipeline::ShaderStage &
Pipeline::ShaderStage::setStages(VkShaderStageFlagBits stages) {
  stages_ = stages;
  return *this;
}

Pipeline::ShaderStage &
Pipeline::ShaderStage::addSpecializationMapEntry(u32 constant_ID, u32 offset,
                                                 u32 size) {
  VkSpecializationMapEntry map_entry;
  map_entry.constantID = constant_ID;
  map_entry.offset = offset;
  map_entry.size = size;
  specialization_map_entries_.emplace_back(map_entry);
  return *this;
}

VkPipelineShaderStageCreateInfo
Pipeline::ShaderStage::create(const ShaderModule &shader_module) const {
  VkPipelineShaderStageCreateInfo info;
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  info.flags = {};
  info.pNext = nullptr;
  info.module = *shader_module;
  info.pName = shader_module.name().data();
  info.stage = stages_;
  info.pSpecializationInfo = nullptr;
  return info;
}

VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(Pipeline::Layout, addFlags,
                                     VkPipelineLayoutCreateFlags,
                                     flags_ |= value)
VENUS_DEFINE_SET_CONFIG_FIELD_METHOD(Pipeline::Layout, addDescriptorSetLayout,
                                     VkDescriptorSetLayout,
                                     set_layouts_.emplace_back(value))
Pipeline::Layout::Config &
Pipeline::Layout::Config::addPushConstantRange(VkShaderStageFlags stage_flags,
                                               uint32_t offset, uint32_t size) {
  VkPushConstantRange range;
  range.offset = offset;
  range.size = size;
  range.stageFlags = stage_flags;
  ranges_.emplace_back(range);
  return *this;
}

Result<Pipeline::Layout>
Pipeline::Layout::Config::create(VkDevice vk_device) const {
  Pipeline::Layout layout;

  VkPipelineLayoutCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  info.pNext = nullptr;
  info.flags = flags_;
  info.pSetLayouts = set_layouts_.data();
  info.setLayoutCount = set_layouts_.size();
  info.pPushConstantRanges = ranges_.data();
  info.pushConstantRangeCount = ranges_.size();

  VENUS_VK_RETURN_BAD_RESULT(
      vkCreatePipelineLayout(vk_device, &info, nullptr, &layout.vk_layout_));

  layout.vk_device_ = vk_device;

#ifdef VENUS_DEBUG
  layout.config_ = *this;
#endif

  return Result<Pipeline::Layout>(std::move(layout));
}

Pipeline::Layout::Layout(Pipeline::Layout &&rhs) noexcept {
  *this = std::move(rhs);
}

Pipeline::Layout::~Layout() noexcept { destroy(); }

Pipeline::Layout &Pipeline::Layout::operator=(Pipeline::Layout &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void Pipeline::Layout::swap(Pipeline::Layout &rhs) {
  VENUS_SWAP_FIELD_WITH_RHS(vk_device_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_layout_);
#ifdef VENUS_DEBUG
  VENUS_SWAP_FIELD_WITH_RHS(config_);
#endif
}

void Pipeline::Layout::destroy() noexcept {
  if (vk_device_ && vk_layout_)
    vkDestroyPipelineLayout(vk_device_, vk_layout_, nullptr);
  vk_device_ = VK_NULL_HANDLE;
  vk_layout_ = VK_NULL_HANDLE;
#ifdef VENUS_DEBUG
  config_ = {};
#endif
}

VkPipelineLayout Pipeline::Layout::operator*() const { return vk_layout_; }

Pipeline::Pipeline(Pipeline &&rhs) noexcept { *this = std::move(rhs); }

Pipeline::~Pipeline() noexcept { destroy(); }

Pipeline &Pipeline::operator=(Pipeline &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void Pipeline::swap(Pipeline &rhs) noexcept {
  VENUS_SWAP_FIELD_WITH_RHS(vk_device_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_pipeline_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_pipeline_cache_);
}

void Pipeline::destroy() noexcept {
  if (vk_device_) {
    if (vk_pipeline_cache_)
      vkDestroyPipelineCache(vk_device_, vk_pipeline_cache_, nullptr);
    if (vk_pipeline_)
      vkDestroyPipeline(vk_device_, vk_pipeline_, nullptr);
  }
  vk_device_ = VK_NULL_HANDLE;
  vk_pipeline_cache_ = VK_NULL_HANDLE;
  vk_pipeline_ = VK_NULL_HANDLE;
}

VkPipeline Pipeline::operator*() const { return vk_pipeline_; }

GraphicsPipeline::VertexInput &
GraphicsPipeline::VertexInput::addBindingDescription(
    u32 binding, u32 stride, VkVertexInputRate input_rate) {
  VkVertexInputBindingDescription info{};
  info.binding = binding;
  info.stride = stride;
  info.inputRate = input_rate;
  binding_descriptions_.emplace_back(info);
  return *this;
}

GraphicsPipeline::VertexInput &
GraphicsPipeline::VertexInput::addAttributeDescription(u32 location,
                                                       u32 binding,
                                                       VkFormat format,
                                                       u32 offset) {
  VkVertexInputAttributeDescription info{};
  info.binding = binding;
  info.location = location;
  info.format = format;
  info.offset = offset;
  attribute_descriptions_.emplace_back(info);
  return *this;
}

VkPipelineVertexInputStateCreateInfo GraphicsPipeline::VertexInput::create(
    VkPipelineVertexInputStateCreateFlags flags) const {
  VkPipelineVertexInputStateCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  info.pNext = nullptr;
  info.flags = flags;
  info.vertexAttributeDescriptionCount = attribute_descriptions_.size();
  info.pVertexAttributeDescriptions = attribute_descriptions_.data();
  info.vertexBindingDescriptionCount = binding_descriptions_.size();
  info.pVertexBindingDescriptions = binding_descriptions_.data();
  return info;
}

GraphicsPipeline::VertexInput GraphicsPipeline::VertexInput::fromVertexLayout(
    const mem::VertexLayout &vertex_layout, u32 binding) {
  GraphicsPipeline::VertexInput info;
  const auto &components = vertex_layout.components();
  for (u32 location = 0; location < components.size(); ++location) {
    info.addAttributeDescription(location, binding, components[location].format,
                                 components[location].offset);
  }
  if (!vertex_layout.components().empty())
    info.addBindingDescription(0, vertex_layout.stride(),
                               VK_VERTEX_INPUT_RATE_VERTEX);
  return info;
}

GraphicsPipeline::Rasterizer::Rasterizer() noexcept {
  info_.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  info_.pNext = nullptr;
}
VENUS_DEFINE_SET_INFO_FIELD_METHOD(GraphicsPipeline::Rasterizer,
                                   setDepthClampEnable, VkBool32,
                                   depthClampEnable)
VENUS_DEFINE_SET_INFO_FIELD_METHOD(GraphicsPipeline::Rasterizer,
                                   setRasterizerDiscardEnable, VkBool32,
                                   rasterizerDiscardEnable);
VENUS_DEFINE_SET_INFO_FIELD_METHOD(GraphicsPipeline::Rasterizer, setPolygonMode,
                                   VkPolygonMode, polygonMode)
VENUS_DEFINE_SET_INFO_FIELD_METHOD(GraphicsPipeline::Rasterizer, setCullMode,
                                   VkCullModeFlags, cullMode)
VENUS_DEFINE_SET_INFO_FIELD_METHOD(GraphicsPipeline::Rasterizer, setFrontFace,
                                   VkFrontFace, frontFace)
VENUS_DEFINE_SET_INFO_FIELD_METHOD(GraphicsPipeline::Rasterizer,
                                   setDepthBiasEnable, VkBool32,
                                   depthBiasEnable)
VENUS_DEFINE_SET_INFO_FIELD_METHOD(GraphicsPipeline::Rasterizer,
                                   setDepthBiasConstantFactor, f32,
                                   depthBiasConstantFactor)
VENUS_DEFINE_SET_INFO_FIELD_METHOD(GraphicsPipeline::Rasterizer,
                                   setDepthBiasClamp, f32, depthBiasClamp)
VENUS_DEFINE_SET_INFO_FIELD_METHOD(GraphicsPipeline::Rasterizer,
                                   setDepthBiasSlopeFactor, f32,
                                   depthBiasSlopeFactor)
VENUS_DEFINE_SET_INFO_FIELD_METHOD(GraphicsPipeline::Rasterizer, setLineWidth,
                                   f32, lineWidth)

VkPipelineRasterizationStateCreateInfo GraphicsPipeline::Rasterizer::create(
    VkPipelineRasterizationStateCreateFlags flags) const {
  auto info = info_;
  info.flags = flags;
  return info;
}

GraphicsPipeline::Multisample GraphicsPipeline::Multisample::none() {
  GraphicsPipeline::Multisample multisample;
  multisample.info_.sampleShadingEnable = VK_FALSE;
  multisample.info_.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisample.info_.minSampleShading = 1.f;
  multisample.info_.alphaToCoverageEnable = VK_FALSE;
  multisample.info_.alphaToOneEnable = VK_FALSE;
  return multisample;
}

GraphicsPipeline::Multisample::Multisample() {
  info_.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  info_.pNext = nullptr;
}
VENUS_DEFINE_SET_INFO_FIELD_METHOD(GraphicsPipeline::Multisample,
                                   setRasterizationSamples,
                                   VkSampleCountFlagBits, rasterizationSamples)
VENUS_DEFINE_SET_INFO_FIELD_METHOD(GraphicsPipeline::Multisample,
                                   setSampleShadingEnable, VkBool32,
                                   sampleShadingEnable)
VENUS_DEFINE_SET_INFO_FIELD_METHOD(GraphicsPipeline::Multisample,
                                   setMinSampleShading, f32, minSampleShading)
VENUS_DEFINE_SET_INFO_FIELD_METHOD(GraphicsPipeline::Multisample,
                                   setAlphaToCoverageEnable, VkBool32,
                                   alphaToCoverageEnable)
VENUS_DEFINE_SET_INFO_FIELD_METHOD(GraphicsPipeline::Multisample,
                                   setAlphaToOneEnable, VkBool32,
                                   alphaToOneEnable)
VENUS_DEFINE_SET_FIELD_METHOD(GraphicsPipeline::Multisample, addSampleMask,
                              VkSampleMask, sample_masks_.emplace_back(value))

VkPipelineMultisampleStateCreateInfo GraphicsPipeline::Multisample::create(
    VkPipelineMultisampleStateCreateFlags flags) const {
  auto info = info_;
  info.flags = flags;
  info.pSampleMask = sample_masks_.data();
  return info;
}

GraphicsPipeline::ColorBlend GraphicsPipeline::ColorBlend::none() {

  VkPipelineColorBlendAttachmentState attachment{};
  attachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  attachment.blendEnable = VK_FALSE;

  GraphicsPipeline::ColorBlend blend;
  blend.color_blend_attachments_.clear();
  blend.color_blend_attachments_.emplace_back(attachment);
  return blend;
}

GraphicsPipeline::ColorBlend GraphicsPipeline::ColorBlend::additive() {

  VkPipelineColorBlendAttachmentState attachment{};
  attachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  attachment.blendEnable = VK_FALSE;
  attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
  attachment.colorBlendOp = VK_BLEND_OP_ADD;
  attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  attachment.alphaBlendOp = VK_BLEND_OP_ADD;

  GraphicsPipeline::ColorBlend blend;
  blend.color_blend_attachments_.clear();
  blend.color_blend_attachments_.emplace_back(attachment);
  return blend;
}

GraphicsPipeline::ColorBlend GraphicsPipeline::ColorBlend::alphaBlend() {
  VkPipelineColorBlendAttachmentState attachment{};
  attachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  attachment.blendEnable = VK_TRUE;
  attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  attachment.colorBlendOp = VK_BLEND_OP_ADD;
  attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  attachment.alphaBlendOp = VK_BLEND_OP_ADD;

  GraphicsPipeline::ColorBlend blend;
  blend.color_blend_attachments_.clear();
  blend.color_blend_attachments_.emplace_back(attachment);
  return blend;
}

GraphicsPipeline::ColorBlend::ColorBlend() noexcept {
  info_.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  info_.pNext = nullptr;
}

GraphicsPipeline::ColorBlend &
GraphicsPipeline::ColorBlend::setBlendingLogicOp(VkLogicOp op) {
  info_.logicOp = op;
  if (color_blend_attachments_.empty())
    color_blend_attachments_.emplace_back();
  return *this;
}

GraphicsPipeline::ColorBlend &
GraphicsPipeline::ColorBlend::setBlendingConstants(
    const std::array<f32, 4> &constants) {
  for (int i = 0; i < 4; ++i)
    info_.blendConstants[i] = constants[i];
  return *this;
}

VkPipelineColorBlendStateCreateInfo GraphicsPipeline::ColorBlend::create(
    VkPipelineColorBlendStateCreateFlags flags) const {
  auto info = info_;
  info.attachmentCount = color_blend_attachments_.size();
  info.pAttachments = color_blend_attachments_.data();
  return info;
}

GraphicsPipeline::DepthStencil GraphicsPipeline::DepthStencil::none() {
  GraphicsPipeline::DepthStencil depth_stencil;
  depth_stencil.info_.depthTestEnable = VK_FALSE;
  depth_stencil.info_.depthWriteEnable = VK_FALSE;
  depth_stencil.info_.depthCompareOp = VK_COMPARE_OP_NEVER;
  depth_stencil.info_.depthBoundsTestEnable = VK_FALSE;
  depth_stencil.info_.stencilTestEnable = VK_FALSE;
  depth_stencil.info_.minDepthBounds = 0.f;
  depth_stencil.info_.maxDepthBounds = 1.f;
  return depth_stencil;
}

GraphicsPipeline::DepthStencil
GraphicsPipeline::DepthStencil::depth(bool depth_write, VkCompareOp op) {
  GraphicsPipeline::DepthStencil depth_stencil;
  depth_stencil.info_.depthTestEnable = VK_TRUE;
  depth_stencil.info_.depthWriteEnable = depth_write;
  depth_stencil.info_.depthCompareOp = op;
  depth_stencil.info_.depthBoundsTestEnable = VK_FALSE;
  depth_stencil.info_.stencilTestEnable = VK_FALSE;
  depth_stencil.info_.minDepthBounds = 0.f;
  depth_stencil.info_.maxDepthBounds = 1.f;
  return depth_stencil;
}

GraphicsPipeline::DepthStencil::DepthStencil() noexcept {
  info_.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  info_.pNext = nullptr;
}

VkPipelineDepthStencilStateCreateInfo GraphicsPipeline::DepthStencil::create(
    VkPipelineDepthStencilStateCreateFlags flags) const {
  auto info = info_;
  info.flags = flags;
  return info;
}

GraphicsPipeline::Config::Config() noexcept {
  input_assembly_.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly_.pNext = nullptr;
  input_assembly_.flags = {};

  viewport_.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_.pNext = nullptr;
  viewport_.flags = {};

  dynamic_.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_.pNext = nullptr;
  dynamic_.flags = {};

  tessellation_.sType =
      VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
  tessellation_.pNext = nullptr;
  tessellation_.flags = {};

  rendering_.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
  rendering_.pNext = nullptr;
}

GraphicsPipeline::Config &GraphicsPipeline::Config::addShaderStage(
    VkPipelineShaderStageCreateInfo shader_stage) {
  stages_.emplace_back(shader_stage);
  return *this;
}

GraphicsPipeline::Config &GraphicsPipeline::Config::setVertexInputState(
    const mem::VertexLayout &vertex_layout) {
  vertex_input_ =
      GraphicsPipeline::VertexInput::fromVertexLayout(vertex_layout);
  return *this;
}

GraphicsPipeline::Config &
GraphicsPipeline::Config::setInputTopology(VkPrimitiveTopology topology,
                                           VkBool32 primitive_restart_enable) {
  input_assembly_.flags = {};
  input_assembly_.topology = topology;
  input_assembly_.primitiveRestartEnable = primitive_restart_enable;
  return *this;
}

GraphicsPipeline::Config &GraphicsPipeline::Config::setTesselationControlPoints(
    u32 patch_control_points) {
  tessellation_.flags = {};
  tessellation_.pNext = nullptr;
  tessellation_.patchControlPoints = patch_control_points;
  return *this;
}

GraphicsPipeline::Config &GraphicsPipeline::Config::setRasterizationState(
    const GraphicsPipeline::Rasterizer &rasterizer_state) {
  rasterization_ = rasterizer_state;
  return *this;
}

GraphicsPipeline::Config &GraphicsPipeline::Config::setMultisampleState(
    const GraphicsPipeline::Multisample &multisample) {
  multisample_ = multisample;
  return *this;
}

GraphicsPipeline::Config &GraphicsPipeline::Config::disableMultisampling() {
  multisample_ = GraphicsPipeline::Multisample::none();
  return *this;
}

GraphicsPipeline::Config &GraphicsPipeline::Config::setColorBlend(
    const GraphicsPipeline::ColorBlend &color_blend) {
  color_blend_ = color_blend;
  return *this;
}

GraphicsPipeline::Config &GraphicsPipeline::Config::disableBlending() {
  color_blend_ = GraphicsPipeline::ColorBlend::none();
  return *this;
}

GraphicsPipeline::Config &GraphicsPipeline::Config::setDepthStencilState(
    const GraphicsPipeline::DepthStencil &depth_stencil) {
  depth_stencil_ = depth_stencil;
  return *this;
}

GraphicsPipeline::Config &
GraphicsPipeline::Config::enableDepthTest(bool depth_write, VkCompareOp op) {
  depth_stencil_ = GraphicsPipeline::DepthStencil::depth(depth_write, op);
  return *this;
}

GraphicsPipeline::Config &
GraphicsPipeline::Config::setColorAttachmentFormat(VkFormat format) {
  color_attachment_formats_ = {format};
  return *this;
}

GraphicsPipeline::Config &
GraphicsPipeline::Config::setDepthFormat(VkFormat format) {
  rendering_.depthAttachmentFormat = format;
  return *this;
}

GraphicsPipeline::Config &GraphicsPipeline::Config::setViewportAndDynamicStates(
    const VkExtent2D &extent) {
  dynamic_states_.clear();
  viewports_.clear();
  scissors_.clear();

  VkViewport viewport{};
  viewport.height = static_cast<f32>(extent.height);
  viewport.width = static_cast<f32>(extent.width);
  viewport.maxDepth = 1000;
  VkRect2D scissor{};
  scissor.extent = extent;
  viewports_.emplace_back(viewport);
  scissors_.emplace_back(scissor);
  viewport_.pViewports = viewports_.data();
  viewport_.viewportCount = viewports_.size();
  viewport_.pScissors = scissors_.data();
  viewport_.scissorCount = scissors_.size();
  dynamic_states_.emplace_back(VK_DYNAMIC_STATE_VIEWPORT);
  dynamic_states_.emplace_back(VK_DYNAMIC_STATE_SCISSOR);
  dynamic_.pDynamicStates = dynamic_states_.data();
  dynamic_.dynamicStateCount = dynamic_states_.size();
  return *this;
}

GraphicsPipeline::Config
GraphicsPipeline::Config::defaults(const VkExtent2D &viewport_extent) {
  GraphicsPipeline::Config config;
  // input assembly
  config.setInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  // multisampling
  config.disableMultisampling();
  // rasterization
  config.setRasterizationState(GraphicsPipeline::Rasterizer()
                                   .setPolygonMode(VK_POLYGON_MODE_FILL)
                                   .setCullMode(VK_CULL_MODE_BACK_BIT)
                                   .setFrontFace(VK_FRONT_FACE_CLOCKWISE)
                                   .setLineWidth(1.f));
  // color blending
  config.setColorBlend(GraphicsPipeline::ColorBlend::none()
                           .setBlendingLogicOp(VK_LOGIC_OP_NO_OP)
                           .setBlendingConstants({1.f, 1.f, 1.f, 1.f}));
  // depth test
  config.enableDepthTest(true, VK_COMPARE_OP_LESS_OR_EQUAL);
  // render area and dynamic state
  config.setViewportAndDynamicStates(viewport_extent);

  return config;
}

Result<GraphicsPipeline>
GraphicsPipeline::Config::create(VkDevice vk_device,
                                 VkPipelineLayout vk_pipeline_layout,
                                 VkRenderPass vk_renderpass) const {

  auto vertex_input = vertex_input_.create();
  auto rasterization = rasterization_.create();
  auto multisample = multisample_.create();
  auto depth_stencil = depth_stencil_.create();
  auto color_blend = color_blend_.create();

  VkGraphicsPipelineCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  create_info.flags = {};
  create_info.pStages = stages_.data();
  create_info.stageCount = stages_.size();
  create_info.pInputAssemblyState = &input_assembly_;
  create_info.pTessellationState = &tessellation_;
  create_info.pRasterizationState = &rasterization;
  create_info.pMultisampleState = &multisample;
  create_info.pDepthStencilState = &depth_stencil;
  create_info.pDynamicState = &dynamic_;
  create_info.pVertexInputState = &vertex_input;
  create_info.pViewportState = &viewport_;
  create_info.pColorBlendState = &color_blend;
  create_info.pNext = &rendering_;
  create_info.layout = vk_pipeline_layout;
  create_info.renderPass = vk_renderpass;

  GraphicsPipeline pipeline;
  pipeline.vk_device_ = vk_device;
  VENUS_VK_RETURN_BAD_RESULT(
      vkCreateGraphicsPipelines(vk_device, VK_NULL_HANDLE, 1, &create_info,
                                nullptr, &pipeline.vk_pipeline_));

#ifdef VENUS_DEBUG
  pipeline.config_ = *this;
#endif

  return Result<GraphicsPipeline>(std::move(pipeline));
}

} // namespace venus::pipeline
