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

#pragma once

#include <venus/core/device.h>
#include <venus/io/swapchain.h>
#include <venus/mem/layout.h>
#include <venus/pipeline/shader_module.h>

#include <array>

namespace venus::pipeline {

// The operations recorded in command buffers are processed by the hardware in
// a pipeline. Pipeline objects control the way in which computations are
// performed. Different from OpenGL though, the whole pipeline state is stored
// in a single object (In OpenGL we have a state machine where we can activate
// pieces separately and switch between resources during execution). In
// Vulkan, each configuration will require its own pipeline object (for
// example, if we want to switch between shaders, we need to prepare the whole
// pipeline for the new set of shaders).
// The computations executed inside the pipeline are performed by shaders.
// Shaders are represented by Shader Modules and must be provided to Vulkan as
// SPIR-V assembly code. A single module may contain code for multiple shader
// stages.
// The interface between shader stages and shader resources is specified
// through pipeline layouts (for example, the same address needs to be used in
// shaders).
// There are three types of pipelines:
// - Graphics pipelines
//    Are used for drawing when binded to the command buffer before recording
//    a drawing command. Can be bounded only inside render passes.
// - Compute pipelines
//    Consisted of a single compute shader stage, compute pipelines are used
//    to perform mathematical operations.
// - Ray Tracing pipelines
//    Ray tracing pipelines utilize a dedicated set of shader stages, distinct
//    from the traditional vertex/geometry/fragment stages.
/// Pipeline Interface
class Pipeline {
public:
  /// Builder for shader stage create info.
  struct ShaderStage {
    /// \param stage Pipeline stage.
    ShaderStage &setStages(VkShaderStageFlagBits stages);
    /// \param constant_ID
    /// \param offset
    /// \param size
    ShaderStage &addSpecializationMapEntry(u32 constant_ID, u32 offset,
                                           u32 size);

    HERMES_NODISCARD VkPipelineShaderStageCreateInfo
    build(const ShaderModule &shader_module) const;

  private:
    VkShaderStageFlagBits stages_{};
    std::vector<VkSpecializationMapEntry> specialization_map_entries_;
    std::size_t specialization_data_size_{0};
    const void *specialization_data_{nullptr};

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
    friend struct hermes::DebugTraits<Pipeline::ShaderStage>;
#endif
  };
  /// Pipeline layout.
  class Layout {
  public:
    struct Config {
      Config &addFlags(VkPipelineLayoutCreateFlags flags);
      /// \brief
      /// \param vk_set_layout
      /// \note Set index is assumed to follow the order descriptor set layouts
      ///       are added.
      /// \return
      Config &addDescriptorSetLayout(VkDescriptorSetLayout vk_set_layout);
      Config &addPushConstantRange(VkShaderStageFlags stage_flags,
                                   uint32_t offset, uint32_t size);
      const std::vector<VkDescriptorSetLayout> &descriptorSetLayouts() const;
      const std::vector<VkPushConstantRange> &ranges();

      Result<Layout> build(VkDevice vk_device) const;

    private:
      VkPipelineLayoutCreateFlags flags_{};
      std::vector<VkPushConstantRange> ranges_;
      std::vector<VkDescriptorSetLayout> set_layouts_;

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
      friend struct hermes::DebugTraits<Pipeline::Layout::Config>;
#endif
    };

    VENUS_DECLARE_RAII_FUNCTIONS(Layout)

    void swap(Layout &rhs);
    void destroy() noexcept;
    VkPipelineLayout operator*() const;

  private:
    VkPipelineLayout vk_layout_{VK_NULL_HANDLE};
    VkDevice vk_device_{VK_NULL_HANDLE};

#ifdef VENUS_DEBUG
    Config config_;
#endif

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
    friend struct hermes::DebugTraits<Pipeline::Layout>;
#endif
  };

  VENUS_DECLARE_RAII_FUNCTIONS(Pipeline);

  void destroy() noexcept;
  void swap(Pipeline &rhs) noexcept;
  VkPipeline operator*() const;
  VkPipelineCache cache() const;

protected:
  /// Base config for pipelines.
  template <typename Derived> struct Setup {
    /// \param shader_stage
    Derived &addShaderStage(VkPipelineShaderStageCreateInfo shader_stage);

  protected:
    // shaders
    std::vector<VkPipelineShaderStageCreateInfo> stages_;
  };

  VkDevice vk_device_{VK_NULL_HANDLE};
  VkPipeline vk_pipeline_{VK_NULL_HANDLE};
  VkPipelineCache vk_pipeline_cache_{VK_NULL_HANDLE};

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
  friend struct hermes::DebugTraits<Pipeline>;
#endif
};

VENUS_DEFINE_SETUP_METHOD(Pipeline, addShaderStage,
                          VkPipelineShaderStageCreateInfo,
                          stages_.emplace_back(value))

/// Specialized pipeline for compute.
class ComputePipeline : public Pipeline {};

/// Specialized pipeline for graphics.
class GraphicsPipeline : public Pipeline {
public:
  /// Builder for Vertex Input state configuration.
  struct VertexInput {
    /// \param binding
    /// \param stride
    /// \param
    VertexInput &addBindingDescription(u32 binding, u32 stride,
                                       VkVertexInputRate input_rate);
    /// \param location
    /// \param binding
    /// \param format
    /// \param offset
    VertexInput &addAttributeDescription(u32 location, u32 binding,
                                         VkFormat format, u32 offset);
    /// \brief Create an info from a given vertex layout.
    /// \param vertex_layout
    /// \param binding
    HERMES_NODISCARD static VertexInput
    fromVertexLayout(const mem::VertexLayout &vertex_layout, u32 binding = 0);

    /// \brief Generate a create info object from this.
    /// \param flags [def={}] create flags.
    HERMES_NODISCARD VkPipelineVertexInputStateCreateInfo
    build(VkPipelineVertexInputStateCreateFlags flags = {}) const;

  private:
    // vertex input
    std::vector<VkVertexInputBindingDescription> binding_descriptions_;
    std::vector<VkVertexInputAttributeDescription> attribute_descriptions_;

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
    friend struct hermes::DebugTraits<GraphicsPipeline::VertexInput>;
#endif
  };
  /// Builder for Rasterization state configuration.
  struct Rasterizer {
    Rasterizer() noexcept;
    Rasterizer &setDepthClampEnable(VkBool32 depth_clamp_enable);
    Rasterizer &setRasterizerDiscardEnable(VkBool32 rasterizer_discard_enable);
    Rasterizer &setPolygonMode(VkPolygonMode polygon_mode);
    Rasterizer &setCullMode(VkCullModeFlags cull_mode);
    Rasterizer &setFrontFace(VkFrontFace front_face);
    Rasterizer &setDepthBiasEnable(VkBool32 depth_bias_enable);
    Rasterizer &setDepthBiasConstantFactor(f32 depth_bias_constant_factor);
    Rasterizer &setDepthBiasClamp(f32 depth_bias_clamp);
    Rasterizer &setDepthBiasSlopeFactor(f32 depth_bias_slope_factor);
    Rasterizer &setLineWidth(f32 line_width);

    HERMES_NODISCARD VkPipelineRasterizationStateCreateInfo
    build(VkPipelineRasterizationStateCreateFlags flags = {}) const;

  private:
    VkPipelineRasterizationStateCreateInfo info_{};

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
    friend struct hermes::DebugTraits<
        venus::pipeline::GraphicsPipeline::Rasterizer>;
#endif
  };
  /// Builder for Multisample state configuration.
  struct Multisample {
    /// \brief Sets 1 sample per pixel.
    static Multisample none();

    Multisample();
    Multisample &setRasterizationSamples(VkSampleCountFlagBits samples);
    Multisample &setSampleShadingEnable(VkBool32 sample_shading_enable);
    Multisample &setMinSampleShading(f32 min_sample);
    Multisample &addSampleMask(VkSampleMask sample_mask);
    Multisample &setAlphaToCoverageEnable(VkBool32 alpha_to_coverage_enable);
    Multisample &setAlphaToOneEnable(VkBool32 alpha_to_one_enable);

    HERMES_NODISCARD VkPipelineMultisampleStateCreateInfo
    build(VkPipelineMultisampleStateCreateFlags flags = {}) const;

  private:
    VkPipelineMultisampleStateCreateInfo info_{};
    std::vector<VkSampleMask> sample_masks_;

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
    friend struct hermes::DebugTraits<GraphicsPipeline::Multisample>;
#endif
  };
  /// Builder for Color Blend state configuration.
  struct ColorBlend {
    static ColorBlend none();
    static ColorBlend additive();
    static ColorBlend alphaBlend();

    ColorBlend() noexcept;
    /// \param op Logical operation.
    ColorBlend &setBlendingLogicOp(VkLogicOp op);
    /// \param constants Values for the corresponding RGBA components.
    ColorBlend &setBlendingConstants(const std::array<f32, 4> &constants);

    HERMES_NODISCARD
    VkPipelineColorBlendStateCreateInfo
    build(VkPipelineColorBlendStateCreateFlags flags = {}) const;

  private:
    VkPipelineColorBlendStateCreateInfo info_{};
    std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments_;

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
    friend struct hermes::DebugTraits<GraphicsPipeline::ColorBlend>;
#endif
  };
  /// Builder for Depth Stencil state configuration.
  struct DepthStencil {

    static DepthStencil none();
    /// \param depth_write Enable/Disable depth write.
    /// \param op Compare operation.
    static DepthStencil depth(bool depth_write, VkCompareOp op);

    DepthStencil() noexcept;
    HERMES_NODISCARD
    VkPipelineDepthStencilStateCreateInfo
    build(VkPipelineDepthStencilStateCreateFlags flags = {}) const;

  private:
    VkPipelineDepthStencilStateCreateInfo info_{};

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
    friend struct hermes::DebugTraits<GraphicsPipeline::DepthStencil>;
#endif
  };

  /// Builder for GraphicsPipeline
  struct Config : public Pipeline::Setup<Config> {

    static Config defaults(const VkExtent2D &viewport_extent);

    static Config forDynamicRendering(const io::Swapchain &swapchain);

    Config() noexcept;

    // vertex input state

    /// \param vertex_layout
    Config &setVertexInputState(const mem::VertexLayout &vertex_layout);

    // input assembly state

    /// \param topology Input assembly topology.
    /// \param primitive_restart_enable [def=false]
    Config &setInputTopology(VkPrimitiveTopology topology,
                             VkBool32 primitive_restart_enable = VK_FALSE);

    // tesselation state

    /// \param patch_control_points
    Config &setTesselationControlPoints(u32 patch_control_points);

    // rasterization state

    /// \param info Rasterization state create info.
    Config &setRasterizationState(const Rasterizer &rasterizer_state);

    // multisampling

    /// \param info Multisample state create info.
    Config &setMultisampleState(const Multisample &multisample);
    Config &disableMultisampling();

    // color blend

    Config &setColorBlend(const ColorBlend &color_blend);
    Config &disableBlending();

    // rendering

    /// \param format Attachment color format.
    Config &setColorAttachmentFormat(VkFormat format);
    /// \param format Depth format.
    Config &setDepthFormat(VkFormat format);

    // depth test

    /// \param info Depth stencil state create info.
    Config &setDepthStencilState(const DepthStencil &depth_stencil);
    /// \param depth_write Enable/Disable depth write.
    /// \param op Compare operation.
    Config &enableDepthTest(bool depth_write, VkCompareOp op);

    // viewport/dynamic states

    /// \brief Sets viewport/scissor for the given extent.
    /// \param extent Viewport/Scissor size.
    Config &setViewportAndDynamicStates(const VkExtent2D &extent);

    Result<GraphicsPipeline> build(VkDevice vk_device,
                                   VkPipelineLayout vk_pipeline_layout,
                                   VkRenderPass vk_renderpass) const;

  private:
    // vertex input
    VertexInput vertex_input_;
    // topology
    VkPipelineInputAssemblyStateCreateInfo input_assembly_{};
    // rasterization
    Rasterizer rasterization_;
    // color blend
    ColorBlend color_blend_;
    // multi-sample
    Multisample multisample_;
    // depth stencil
    DepthStencil depth_stencil_;
    // viewport
    std::vector<VkViewport> viewports_;
    std::vector<VkRect2D> scissors_;
    VkPipelineViewportStateCreateInfo viewport_{};
    // dynamic
    std::vector<VkDynamicState> dynamic_states_;
    VkPipelineDynamicStateCreateInfo dynamic_{};
    // tesselation
    VkPipelineTessellationStateCreateInfo tessellation_{};
    // rendering
    std::vector<VkFormat> color_attachment_formats_{};
    VkPipelineRenderingCreateInfo rendering_{};

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
    friend struct hermes::DebugTraits<GraphicsPipeline::Config>;
#endif
  };

private:
#ifdef VENUS_DEBUG
  Config config_;
#endif

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
  friend struct hermes::DebugTraits<GraphicsPipeline>;
#endif
};

/// Specialized pipeline for ray tracing.
class RayTracingPipeline : public Pipeline {
public:
  struct ShaderGroup {
    ShaderGroup();
    ShaderGroup &setType(VkRayTracingShaderGroupTypeKHR type);
    ShaderGroup &setGeneralShader(u32 shader_stage_index);
    ShaderGroup &setClosestHitShader(u32 shader_stage_index);
    ShaderGroup &setAnyHitShader(u32 shader_stage_index);
    ShaderGroup &setIntersectionShader(u32 shader_stage_index);

    VkRayTracingShaderGroupCreateInfoKHR operator*() const;

  private:
    VkRayTracingShaderGroupCreateInfoKHR info_{};
  };

  /// Builder for RayTracingPipeline
  struct Config : public Pipeline::Setup<Config> {
    /// \param shader_group
    Config &addShaderGroup(const ShaderGroup &shader_group);

    Result<RayTracingPipeline> build(VkDevice vk_device,
                                     VkPipelineLayout vk_pipeline_layout) const;

  private:
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> shader_groups_{};

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
    friend struct hermes::DebugTraits<RayTracingPipeline::Config>;
#endif
  };

  const std::vector<VkRayTracingShaderGroupCreateInfoKHR> &shaderGroups() const;

private:
  std::vector<VkRayTracingShaderGroupCreateInfoKHR> shader_groups_{};

#ifdef VENUS_DEBUG
  Config config_;
#endif

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
  friend struct hermes::DebugTraits<RayTracingPipeline>;
#endif
};

} // namespace venus::pipeline

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
namespace hermes {

template <> struct DebugTraits<venus::pipeline::Pipeline::ShaderStage> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage
  message(const venus::pipeline::Pipeline::ShaderStage &data) {
    return DebugMessage()
        .addTitle("Pipeline Shader Stage")
        .add("stages", VENUS_VK_STRING(VkShaderStageFlagBits, data.stages_))
        .addArray("specialization map entries",
                  data.specialization_map_entries_)
        .add("specialization data size", data.specialization_data_size_)
        .addAddress("specialization data", data.specialization_data_);
  }
};

template <> struct DebugTraits<venus::pipeline::Pipeline::Layout::Config> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage
  message(const venus::pipeline::Pipeline::Layout::Config &data) {
    return DebugMessage()
        .addTitle("Pipeline Layout Config")
        .add("flags", VENUS_VK_STRING(VkPipelineLayoutCreateFlags, data.flags_))
        .addArray("ranges", data.ranges_)
        .addArray("set_layouts", data.set_layouts_);
  }
};

template <> struct DebugTraits<venus::pipeline::Pipeline::Layout> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage message(const venus::pipeline::Pipeline::Layout &data) {
    return DebugMessage()
        .addTitle("Pipeline Layout")
        .add("vk_layout", VENUS_VK_HANDLE_STRING(data.vk_layout_))
        .add("vk_device", VENUS_VK_DISPATCHABLE_HANDLE_STRING(data.vk_device_))
        .add("config", data.config_);
  }
};

template <> struct DebugTraits<venus::pipeline::Pipeline> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage message(const venus::pipeline::Pipeline &data) {
    return DebugMessage()
        .addTitle("Pipeline")
        .add("vk_pipeline", VENUS_VK_HANDLE_STRING(data.vk_pipeline_))
        .add("vk_pipeline_cache",
             VENUS_VK_HANDLE_STRING(data.vk_pipeline_cache_))
        .add("vk_device", VENUS_VK_DISPATCHABLE_HANDLE_STRING(data.vk_device_));
  }
};

template <> struct DebugTraits<venus::pipeline::GraphicsPipeline::VertexInput> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage
  message(const venus::pipeline::GraphicsPipeline::VertexInput &data) {
    return DebugMessage()
        .addTitle("Graphics Pipeline Vertex Input")
        .addArray("binding descriptions", data.binding_descriptions_)
        .addArray("attribute descriptions", data.binding_descriptions_);
  }
};

template <> struct DebugTraits<venus::pipeline::GraphicsPipeline::Rasterizer> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage
  message(const venus::pipeline::GraphicsPipeline::Rasterizer &data) {
    return DebugMessage()
        .addTitle("Graphics Pipeline Rasterizer")
        .add("depth clamp enable", data.info_.depthClampEnable)
        .add("rasterizer discard enable", data.info_.rasterizerDiscardEnable)
        .add("depth bias enable", data.info_.depthBiasEnable)
        .add("depth bias constant factor", data.info_.depthBiasConstantFactor)
        .add("depth bias clamp", data.info_.depthBiasClamp)
        .add("depth bias slope factor", data.info_.depthBiasSlopeFactor)
        .add("line width", data.info_.lineWidth)
        .add("polygon mode",
             VENUS_VK_STRING(VkPolygonMode, data.info_.polygonMode))
        .add("cull mode", VENUS_VK_STRING(VkCullModeFlags, data.info_.cullMode))
        .add("front face", VENUS_VK_STRING(VkFrontFace, data.info_.frontFace));
  }
};

template <> struct DebugTraits<venus::pipeline::GraphicsPipeline::Multisample> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage
  message(const venus::pipeline::GraphicsPipeline::Multisample &data) {
    return DebugMessage()
        .addTitle("Graphics Pipeline Vertex Multisample")
        .add("rasterization samples",
             VENUS_VK_STRING(VkSampleCountFlagBits,
                             data.info_.rasterizationSamples))
        .add("sample shading enable", data.info_.sampleShadingEnable)
        .add("min sample shading", data.info_.minSampleShading)
        .add("pSample mask", data.info_.pSampleMask)
        .add("alpha to coverage enable", data.info_.alphaToCoverageEnable)
        .add("alpha to one enable", data.info_.alphaToOneEnable)
        .addArray("sample masks", data.sample_masks_);
  }
};

template <> struct DebugTraits<venus::pipeline::GraphicsPipeline::ColorBlend> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage
  message(const venus::pipeline::GraphicsPipeline::ColorBlend &data) {
    return DebugMessage()
        .addTitle("Graphics Pipeline Vertex Color Blend")
        .add("logic op enable", data.info_.logicOpEnable)
        .add("logic op", VENUS_VK_STRING(VkLogicOp, data.info_.logicOp))
        .add("attachment count", data.info_.attachmentCount)
        .addArray("color blend attachments", data.color_blend_attachments_);
  }
};

template <>
struct DebugTraits<venus::pipeline::GraphicsPipeline::DepthStencil> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage
  message(const venus::pipeline::GraphicsPipeline::DepthStencil &data) {
    return DebugMessage()
        .addTitle("Graphics Pipeline Vertex Depth Stencil")
        .add("depth test enable", data.info_.depthTestEnable)
        .add("depth write enable", data.info_.depthWriteEnable)
        .add("depth compare op",
             VENUS_VK_STRING(VkCompareOp, data.info_.depthCompareOp))
        .add("depth bounds test enable", data.info_.depthBoundsTestEnable)
        .add("stencil test eblane", data.info_.stencilTestEnable)
        .add("front", data.info_.front)
        .add("back", data.info_.back)
        .add("min depth bounds", data.info_.minDepthBounds)
        .add("max depth bounds", data.info_.maxDepthBounds);
  }
};

template <> struct DebugTraits<venus::pipeline::GraphicsPipeline::Config> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage
  message(const venus::pipeline::GraphicsPipeline::Config &data) {
    return DebugMessage()
        .addTitle("Graphics Pipeline Config")
        .add("vertex input", data.vertex_input_)
        .add("rasterization", data.rasterization_)
        .add("color blend", data.color_blend_)
        .add("multisample", data.multisample_)
        .add("depth stencil", data.depth_stencil_)
        .addFmt("input_assembly = topology {} restartEnable {}",
                string_VkPrimitiveTopology(data.input_assembly_.topology),
                data.input_assembly_.primitiveRestartEnable)
        .addArray("viewports", data.viewports_)
        .addArray("scissors", data.scissors_)
        .addFmt("viewport_ = viewports {} scissors {}",
                data.viewport_.viewportCount, data.viewport_.scissorCount)
        .addArray("color attachment formats", data.color_attachment_formats_)
        .addArray("stages", data.stages_);
  }
};

template <> struct DebugTraits<venus::pipeline::GraphicsPipeline> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage message(const venus::pipeline::GraphicsPipeline &data) {
    return DebugMessage()
        .addTitle("Graphics Pipeline")
        .add("config", data.config_);
  }
};

} // namespace hermes
#endif