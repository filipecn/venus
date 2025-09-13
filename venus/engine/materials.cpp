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

Result<Material> Material_Color::material(const engine::GraphicsDevice &gd) {
  // descriptor set layouts

  pipeline::DescriptorSet::Layout l;
  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      l, pipeline::DescriptorSet::Layout::Config()
             .addLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
                               VK_SHADER_STAGE_VERTEX_BIT)
             .create(**gd));
  auto vk_descriptor_set_layout = *l;

  auto &globals = engine::GraphicsEngine::globals();

  Material m;

  VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
      m,
      Material::Config()
          .setDescriptorSetLayout(std::move(l))
          .setMaterialPipelineConfig(
              Material::Pipeline::Config()
                  .setPipelineConfig(
                      pipeline::GraphicsPipeline::Config::defaults(
                          gd.swapchain().imageExtent())
                          .setVertexInputState(
                              mem::VertexLayout()
                                  .pushComponent(mem::VertexLayout::
                                                     ComponentType::Position,
                                                 VK_FORMAT_R32G32B32A32_SFLOAT)
                                  .pushComponent(
                                      mem::VertexLayout::ComponentType::Color,
                                      VK_FORMAT_R32G32B32A32_SFLOAT))
                          .setColorAttachmentFormat(
                              gd.swapchain().colorFormat())
                          .setDepthFormat(gd.swapchain().depthBuffer().format())
                          .addShaderStage(
                              pipeline::Pipeline::ShaderStage()
                                  .setStages(VK_SHADER_STAGE_VERTEX_BIT)
                                  .create(globals.shaders.vert_color))
                          .addShaderStage(
                              pipeline::Pipeline::ShaderStage()
                                  .setStages(VK_SHADER_STAGE_FRAGMENT_BIT)
                                  .create(globals.shaders.frag_color)))
                  .setPipelineLayoutConfig(
                      pipeline::Pipeline::Layout::Config()
                          .addDescriptorSetLayout(vk_descriptor_set_layout)))
          .create(**gd, *gd.renderpass()));

  return Result<Material>(std::move(m));
}

} // namespace venus::scene
