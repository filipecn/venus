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

/// \file   materials.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30

#include <venus/engine/materials.h>

#include <venus/engine/graphics_engine.h>

namespace venus::scene {

Result<Material> Material_Test::material(const engine::GraphicsDevice &gd) {
  pipeline::DescriptorSet::Layout l;
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      l, pipeline::DescriptorSet::Layout::Config()
             .addLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
                               VK_SHADER_STAGE_VERTEX_BIT)
             .create(**gd));

  auto &globals = engine::GraphicsEngine::globals();

  auto pipeline_layout_config =
      pipeline::Pipeline::Layout::Config()
          .addDescriptorSetLayout(globals.descriptors.scene_data_layout)
          .addDescriptorSetLayout(*l)
          .addPushConstantRange(
              VK_SHADER_STAGE_VERTEX_BIT, 0,
              sizeof(
                  engine::GraphicsEngine::Globals::Types::DrawPushConstants));

  auto pipeline_config =
      pipeline::GraphicsPipeline::Config::forDynamicRendering(gd.swapchain())
          .setVertexInputState(mem::VertexLayout().pushComponent(
              mem::VertexLayout::ComponentType::Position,
              VK_FORMAT_R32G32B32_SFLOAT)
                               //.pushComponent(
                               //    mem::VertexLayout::ComponentType::Color,
                               //    VK_FORMAT_R32G32B32A32_SFLOAT)
                               )
          .addShaderStage(pipeline::Pipeline::ShaderStage()
                              .setStages(VK_SHADER_STAGE_VERTEX_BIT)
                              .create(globals.shaders.vert_test))
          .addShaderStage(pipeline::Pipeline::ShaderStage()
                              .setStages(VK_SHADER_STAGE_FRAGMENT_BIT)
                              .create(globals.shaders.frag_flat_color));

  VENUS_DECLARE_OR_RETURN_BAD_RESULT(
      Material, m,
      Material::Config()
          .setDescriptorSetLayout(std::move(l))
          .setMaterialPipelineConfig(
              Material::Pipeline::Config()
                  .setPipelineConfig(pipeline_config)
                  .setPipelineLayoutConfig(pipeline_layout_config))
          .create(**gd, *gd.renderpass()));

  return Result<Material>(std::move(m));
}

Result<Material::Instance>
Material_Test::write(pipeline::DescriptorAllocator &allocator,
                     const Material *material) {
  Material::Instance instance;

  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      instance,
      Material::Instance::Config().setMaterial(material).create(allocator))

  // descriptor_writer_.clear();
  // descriptor_writer_.writeBuffer(
  //     0, resources.data_buffer, sizeof(Material_Test::Data),
  //     resources.data_buffer_offset, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
  // descriptor_writer_.update(instance.descriptorSet());

  return Result<Material::Instance>(std::move(instance));
}

Result<Material>
Material_BindlessTest::material(const engine::GraphicsDevice &gd) {
  pipeline::DescriptorSet::Layout l;
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      l, pipeline::DescriptorSet::Layout::Config()
             .addLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
                               VK_SHADER_STAGE_VERTEX_BIT)
             .create(**gd));

  auto &globals = engine::GraphicsEngine::globals();

  auto pipeline_layout_config =
      pipeline::Pipeline::Layout::Config()
          .addDescriptorSetLayout(globals.descriptors.scene_data_layout)
          //.addDescriptorSetLayout(*l)
          .addPushConstantRange(
              VK_SHADER_STAGE_VERTEX_BIT, 0,
              sizeof(
                  engine::GraphicsEngine::Globals::Types::DrawPushConstants));

  auto pipeline_config =
      pipeline::GraphicsPipeline::Config::forDynamicRendering(gd.swapchain())
          .addShaderStage(pipeline::Pipeline::ShaderStage()
                              .setStages(VK_SHADER_STAGE_VERTEX_BIT)
                              .create(globals.shaders.vert_bindless_test))
          .addShaderStage(pipeline::Pipeline::ShaderStage()
                              .setStages(VK_SHADER_STAGE_FRAGMENT_BIT)
                              .create(globals.shaders.frag_flat_color));

  Material m;
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      m, Material::Config()
             //.setDescriptorSetLayout(std::move(l))
             .setMaterialPipelineConfig(
                 Material::Pipeline::Config()
                     .setPipelineConfig(pipeline_config)
                     .setPipelineLayoutConfig(pipeline_layout_config))
             .create(**gd, *gd.renderpass()));

  return Result<Material>(std::move(m));
}

} // namespace venus::scene
