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

#include <hermes/core/ref.h>

namespace venus::scene {

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
      Config &setMaterial(const Material *material);
      Result<Instance> create(pipeline::DescriptorAllocator &allocator) const;

    private:
      const Material *material_{nullptr};
      VkDescriptorSetLayout vk_descriptor_set_layout_{VK_NULL_HANDLE};
    };

    VENUS_DECLARE_RAII_FUNCTIONS(Instance)

    void swap(Instance &rhs);
    void destroy() noexcept;

    const Material *material() const;
    const pipeline::DescriptorSet &descriptorSet() const;
    const pipeline::GraphicsPipeline &pipeline() const;
    const pipeline::Pipeline::Layout &pipelineLayout() const;

  private:
    const Material *material_{nullptr};
    pipeline::DescriptorSet descriptor_set_;

    VENUS_to_string_FRIEND(Instance);
  };

  struct Writer {
    virtual Result<Instance> write(pipeline::DescriptorAllocator &allocator,
                                   const Material *material) = 0;

  protected:
    pipeline::DescriptorWriter descriptor_writer_;
  };

  struct Pipeline {
    struct Config {
      Config &
      setPipelineConfig(const pipeline::GraphicsPipeline::Config &config);
      Config &
      setPipelineLayoutConfig(const pipeline::Pipeline::Layout::Config &config);

      Result<Material::Pipeline> create(VkDevice vk_device,
                                        VkRenderPass vk_renderpass) const;

    private:
      pipeline::GraphicsPipeline::Config pipeline_config_;
      pipeline::Pipeline::Layout::Config layout_config_;

      VENUS_to_string_FRIEND(Config);
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

    VENUS_to_string_FRIEND(Pipeline);
  };

  struct Config {
    Config &setDescriptorSetLayout(
        pipeline::DescriptorSet::Layout &&descriptor_set_layout);
    Config &setMaterialPipelineConfig(const Pipeline::Config &config);

    Result<Material> create(VkDevice vk_device,
                            VkRenderPass vk_renderpass) const;

  private:
    mutable pipeline::DescriptorSet::Layout descriptor_set_layout_;
    Pipeline::Config pipeline_config_;

    VENUS_to_string_FRIEND(Config);
  };

  VENUS_DECLARE_RAII_FUNCTIONS(Material)

  void destroy() noexcept;
  void swap(Material &rhs);

  const pipeline::DescriptorSet::Layout &descriptorSetLayout() const;
  const Pipeline &pipeline() const;

protected:
  Pipeline pipeline_;
  pipeline::DescriptorSet::Layout descriptor_set_layout_;

#ifdef VENUS_DEBUG
  Config config_;
#endif

  VENUS_to_string_FRIEND(Material);
};

} // namespace venus::scene
