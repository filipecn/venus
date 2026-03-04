/* Copyright (c) 2026, FilipeCN.
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

/// \file   sky_background.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2026-02-17

#include <venus/scene/models/sky_background.h>

#include <venus/engine/graphics_engine.h>
#include <venus/engine/shapes.h>

#include <hermes/geometry/transform.h>

namespace venus::scene::models {

Result<Material> createSkyMaterial(const engine::GraphicsDevice &gd) {
  // setup shaders
  std::filesystem::path shaders_path(VENUS_SHADERS_PATH);
  pipeline::ShaderModule vert;
  pipeline::ShaderModule frag;
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      vert, pipeline::ShaderModule::Config()
                .setEntryFuncName("main")
                .fromSpvFile(shaders_path / "sky_background.vert.spv")
                .build(**gd));
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      frag, pipeline::ShaderModule::Config()
                .setEntryFuncName("main")
                .fromSpvFile(shaders_path / "sky_background.frag.spv")
                .build(**gd));

  auto pipeline_layout_config =
      pipeline::Pipeline::Layout::Config().addPushConstantRange(
          VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(hermes::geo::Transform));

  auto pipeline_config =
      pipeline::GraphicsPipeline::Config::forDynamicRendering(gd.swapchain())
          .enableDepthTest(true, VK_COMPARE_OP_ALWAYS)
          .addShaderStage(pipeline::Pipeline::ShaderStage()
                              .setStages(VK_SHADER_STAGE_VERTEX_BIT)
                              .build(vert))
          .addShaderStage(pipeline::Pipeline::ShaderStage()
                              .setStages(VK_SHADER_STAGE_FRAGMENT_BIT)
                              .build(frag));

  VENUS_DECLARE_OR_RETURN_BAD_RESULT(
      Material, m,
      Material::Config()
          .setMaterialPipelineConfig(
              Material::Pipeline::Config()
                  .setPipelineConfig(pipeline_config)
                  .setPipelineLayoutConfig(pipeline_layout_config))
          .build(**gd, *gd.renderpass()));

  return Result<Material>(std::move(m));
}

Result<SkyBackground>
SkyBackground::Config::build(const engine::GraphicsDevice &gd) const {
  SkyBackground cg;

  // setup material
  VENUS_DECLARE_SHARED_PTR_FROM_RESULT_OR_RETURN_BAD_RESULT(
      Material, material, createSkyMaterial(gd));
  cg.material_ = material;

  Model::Shape shape;
  shape.bounds = {};
  shape.index_count = 3;
  shape.index_base = 0;
  shape.vertex_count = 0;

  VENUS_DECLARE_SHARED_PTR_FROM_RESULT_OR_RETURN_BAD_RESULT(
      Material::Instance, instance,
      Material::Instance::Config()
          .setWritePushConstants(
              VK_SHADER_STAGE_VERTEX_BIT,
              [](hermes::mem::Block &block,
                 const PushConstantsContext &ctx) -> VeResult {
                auto inv_viewproj = hermes::math::transpose(
                    (ctx.inv_view * ctx.inv_projection).matrix());
                VENUS_RETURN_BAD_HE_RESULT(
                    block.resize(sizeof(hermes::geo::Transform)));
                VENUS_RETURN_BAD_HE_RESULT(block.copy(&inv_viewproj));
                return VeResult::noError();
              })
          .setMaterial(cg.material_)
          .build(
              venus::engine::GraphicsEngine::globals().descriptors.allocator()))
  shape.material_instance = instance;
  cg.shapes_.emplace_back(shape);

  return Result<SkyBackground>(std::move(cg));
}

SkyBackground::SkyBackground(SkyBackground &&rhs) noexcept {
  *this = std::move(rhs);
}

SkyBackground::~SkyBackground() noexcept { destroy(); }

SkyBackground &SkyBackground::operator=(SkyBackground &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void SkyBackground::destroy() noexcept {
  material_.destroy();

  vk_index_buffer_ = VK_NULL_HANDLE;
  vk_vertex_buffer_ = VK_NULL_HANDLE;
  vk_index_buffer_ = VK_NULL_HANDLE;
  vk_transform_buffer_ = VK_NULL_HANDLE;
  vk_vertex_buffer_address_ = 0;
  vk_index_buffer_address_ = 0;
  vk_transform_buffer_address_ = 0;
}

void SkyBackground::swap(SkyBackground &rhs) {
  VENUS_SWAP_FIELD_WITH_RHS(shapes_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_vertex_buffer_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_index_buffer_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_vertex_buffer_address_);
  VENUS_SWAP_FIELD_WITH_RHS(vk_index_buffer_address_);
  VENUS_SWAP_FIELD_WITH_RHS(vertex_layout_);
  VENUS_SWAP_FIELD_WITH_RHS(material_);
}

} // namespace venus::scene::models