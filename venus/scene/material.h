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

/// \file   material.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30
/// \brief  Model material.

#pragma once

#include <venus/pipeline/descriptors.h>
#include <venus/pipeline/pipeline.h>
#include <venus/utils/indexed_handle.h>

#include <hermes/core/ref.h>
#include <hermes/geometry/transform.h>
#include <hermes/storage/block.h>

#include <set>

namespace venus::scene {

struct PushConstantsContext {
  hermes::geo::Transform projection;
  hermes::geo::Transform view;
  hermes::geo::Transform inv_projection;
  hermes::geo::Transform inv_view;
  hermes::geo::Transform proj_view;
  hermes::geo::Transform model;
  hermes::geo::point3 eye;
};

// *****************************************************************************
//                                                                    Material
// *****************************************************************************

/// \brief Base/interface class for materials.
/// This class holds the pipeline of a particular material along with a
/// descriptor set writer, so child classes may update descriptor sets in their
/// own ways.
class Material {
public:
  using Ptr = hermes::Ref<Material>;

  struct Instance {
    using Ptr = hermes::Ref<Instance>;

    struct Config {
      Config &setMaterial(Material::Ptr material);
      Config &addGlobalSetIndex(h_index global_set_index);
      Config &setWritePushConstants(
          VkShaderStageFlags stage_flags,
          const std::function<VeResult(hermes::mem::Block &,
                                       const PushConstantsContext &)> &f);
      Result<Instance> build(pipeline::DescriptorAllocator &allocator) const;

    private:
      Material::Ptr material_;
      std::set<h_index> global_set_indices_;
      VkShaderStageFlags push_constants_stage_flags_;
      std::function<VeResult(hermes::mem::Block &,
                             const PushConstantsContext &)>
          write_push_constants_;
    };

    VENUS_DECLARE_RAII_FUNCTIONS(Instance)

    void swap(Instance &rhs);
    void destroy() noexcept;

    Material::Ptr material() const;
    const pipeline::GraphicsPipeline &pipeline() const;
    const pipeline::Pipeline::Layout &pipelineLayout() const;
    bool hasGlobalDescriptors() const;
    std::unordered_map<h_index, std::vector<VkDescriptorSet>>
    localDescriptorSetGroups() const;
    const pipeline::DescriptorSet &localDescriptorSet(h_index set_index) const;
    VeResult writePushConstants(hermes::mem::Block &block,
                                const PushConstantsContext &ctx) const;
    VkShaderStageFlags pushConstantsStageFlags() const;

  private:
    Material::Ptr material_;
    /// set index -> descriptor set object map
    std::unordered_map<h_index, pipeline::DescriptorSet> descriptor_sets_;
    // TODO: this is weird (think in a better way to handle global set
    // descriptors)
    std::set<h_index> global_set_indices_;
    std::function<VeResult(hermes::mem::Block &, const PushConstantsContext &)>
        write_push_constants_;
    VkShaderStageFlags push_constants_stage_flags_{VK_SHADER_STAGE_VERTEX_BIT};

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
    friend struct hermes::DebugTraits<Material::Instance>;
#endif
  };

  struct Writer {
    virtual Result<Instance> write(pipeline::DescriptorAllocator &allocator,
                                   Material::Ptr material);

  protected:
    pipeline::DescriptorWriter descriptor_writer_;
  };

  struct Pipeline {
    struct Config {
      Config &
      setPipelineConfig(const pipeline::GraphicsPipeline::Config &config);
      Config &
      setPipelineLayoutConfig(const pipeline::Pipeline::Layout::Config &config);
      const pipeline::Pipeline::Layout::Config &
      graphicsPipelineLayoutConfig() const;
      Result<Material::Pipeline> build(VkDevice vk_device,
                                       VkRenderPass vk_renderpass) const;

    private:
      pipeline::GraphicsPipeline::Config graphics_pipeline_config_;
      pipeline::Pipeline::Layout::Config graphics_pipeline_layout_config_;

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
      friend struct hermes::DebugTraits<Material::Pipeline::Config>;
#endif
    };

    VENUS_DECLARE_RAII_FUNCTIONS(Pipeline)

    void destroy() noexcept;
    void swap(Pipeline &rhs);

    /// \return The underlying pipeline object.
    HERMES_NODISCARD const pipeline::GraphicsPipeline &operator*() const;
    /// \return The underlying pipeline object.
    HERMES_NODISCARD const pipeline::GraphicsPipeline &pipeline() const;
    /// \return The underlying pipeline layout.
    HERMES_NODISCARD const pipeline::Pipeline::Layout &pipelineLayout() const;
    /// \return The underlying pipeline vulkan object.
    HERMES_NODISCARD VkPipeline vkPipeline() const;

  private:
    pipeline::GraphicsPipeline pipeline_;
    pipeline::Pipeline::Layout pipeline_layout_;

#ifdef VENUS_DEBUG
    Config config_;
#endif

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
    friend struct hermes::DebugTraits<Material::Pipeline>;
#endif
  };

  struct Config {
    Config &setMaterialPipelineConfig(const Pipeline::Config &config);

    Result<Material> build(VkDevice vk_device,
                           VkRenderPass vk_renderpass) const;

  private:
    Pipeline::Config pipeline_config_;

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
    friend struct hermes::DebugTraits<Material::Config>;
#endif
  };

  VENUS_DECLARE_RAII_FUNCTIONS(Material)

  void destroy() noexcept;
  void swap(Material &rhs);

  const Pipeline &pipeline() const;
  void ownDescriptorSetLayout(
      pipeline::DescriptorSet::Layout &&descriptor_set_layout);
  const std::unordered_map<h_index, VkDescriptorSetLayout> &
  descriptorSetLayouts() const;

protected:
  Pipeline pipeline_;
  std::vector<pipeline::DescriptorSet::Layout> owned_descriptor_set_layouts_;
  std::unordered_map<h_index, VkDescriptorSetLayout> vk_descriptor_set_layouts_;
#ifdef VENUS_DEBUG
  Config config_;
#endif

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
  friend struct hermes::DebugTraits<Material>;
#endif
};

} // namespace venus::scene

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
namespace hermes {

template <> struct DebugTraits<venus::scene::Material::Pipeline::Config> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage
  message(const venus::scene::Material::Pipeline::Config &data) {
    return DebugMessage()
        .addTitle("Scene Material Pipeline Config")
        .add("pipeline config", data.graphics_pipeline_config_)
        .add("layout config", data.graphics_pipeline_layout_config_);
  }
};

template <> struct DebugTraits<venus::scene::Material::Pipeline> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage message(const venus::scene::Material::Pipeline &data) {
    return DebugMessage()
        .addTitle("Scene Material Pipeline")
        .add("pipeline", data.pipeline_)
        .add("pipeline layout", data.pipeline_layout_)
        .add("config", data.config_);
  }
};

template <> struct DebugTraits<venus::scene::Material::Config> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage message(const venus::scene::Material::Config &data) {
    return DebugMessage()
        .addTitle("Scene Material Config")
        .add("pipeline config", data.pipeline_config_);
  }
};

template <> struct DebugTraits<venus::scene::Material> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage message(const venus::scene::Material &data) {
    return DebugMessage()
        .addTitle("SCene Material")
        .add("pipeline", data.pipeline_)
        .add("descriptor set layouts", data.vk_descriptor_set_layouts_.size())
        .addArray("owned descriptor set layouts",
                  data.owned_descriptor_set_layouts_)
        .add("config", data.config_);
  }
};

template <> struct DebugTraits<venus::scene::Material::Instance> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage message(const venus::scene::Material::Instance &data) {
    DebugMessage m;
    m.addTitle("Scene Material Instance")
        .add("material", *data.material_)
        .addFmt("Descriptor Sets")
        .pushTab();
    for (const auto &item : data.descriptor_sets_)
      m.addFmt("set index{} -> ds: {}", item.first,
               hermes::to_string(item.second));
    return m;
  }
};

} // namespace hermes

#endif // VENUS_INCLUDE_DEBUG_TRAITS