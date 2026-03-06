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

/// \file   sim_surface_field.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2026-03-04
/// \brief Rendering scalar fields on surfaces.
///
///        This example uses color palettes to paint scalar fields on top of
///        surfaces. At first, colors could just be stored in the vertex buffer
///        as usual, but simulation results can be already on the gpu.
///        Simulation results are commonly stored as scalar fields defined on
///        mesh elements, such as:
///          - faces
///          - vertices
///        Simulation fields change values every frame, while the mesh can be
///        static. Therefore, this exemple considers the simulation data
///        separate from the vertex buffer containing the mesh elements.

#include <venus/app/scene_app.h>
#include <venus/engine/shapes.h>
#include <venus/io/glfw_display.h>
#include <venus/scene/models/cartesian_grid.h>
#include <venus/scene/models/sky_background.h>

class MAT_ScalarField : public venus::scene::Material::Writer {
public:
  using Ptr = hermes::Ref<MAT_ScalarField>;

  struct PushConstants {
    hermes::geo::Transform inv_view_inv_proj;
  };

  struct Data {
    VkDeviceAddress mesh_buffer;
    VkDeviceAddress scalar_buffer;
  };

  struct Resources {
    VkBuffer data_buffer;
    u32 data_buffer_offset;
  };

  static venus::Result<venus::scene::Material>
  material(const venus::engine::GraphicsDevice &gd) {
    using namespace venus;
    // setup shaders
    std::filesystem::path shaders_path(VENUS_SHADERS_PATH);
    VENUS_DECLARE_OR_RETURN_BAD_RESULT(
        pipeline::ShaderModule, vert,
        pipeline::ShaderModule::Config()
            .setEntryFuncName("main")
            .fromSpvFile(shaders_path / "scalar_field.vert.spv")
            .build(**gd));
    VENUS_DECLARE_OR_RETURN_BAD_RESULT(
        pipeline::ShaderModule, frag,
        pipeline::ShaderModule::Config()
            .setEntryFuncName("main")
            .fromSpvFile(shaders_path / "scalar_field.frag.spv")
            .build(**gd));

    VENUS_DECLARE_OR_RETURN_BAD_RESULT(
        pipeline::DescriptorSet::Layout, l,
        pipeline::DescriptorSet::Layout::Config()
            .addLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
                              VK_SHADER_STAGE_VERTEX_BIT)
            .build(**gd));

    auto pipeline_layout_config =
        pipeline::Pipeline::Layout::Config()
            .addDescriptorSetLayout(*l)
            .addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, 0,
                                  sizeof(PushConstants));

    auto pipeline_config =
        pipeline::GraphicsPipeline::Config::forDynamicRendering(gd.swapchain())
            .addShaderStage(pipeline::Pipeline::ShaderStage()
                                .setStages(VK_SHADER_STAGE_VERTEX_BIT)
                                .build(vert))
            .addShaderStage(pipeline::Pipeline::ShaderStage()
                                .setStages(VK_SHADER_STAGE_FRAGMENT_BIT)
                                .build(frag));

    VENUS_DECLARE_OR_RETURN_BAD_RESULT(
        scene::Material, m,
        scene::Material::Config()
            .setMaterialPipelineConfig(
                scene::Material::Pipeline::Config()
                    .setPipelineConfig(pipeline_config)
                    .setPipelineLayoutConfig(pipeline_layout_config))
            .build(**gd, *gd.renderpass()));

    m.ownDescriptorSetLayout(std::move(l));

    return Result<scene::Material>(std::move(m));
  }

  venus::Result<venus::scene::Material::Instance>
  write(venus::pipeline::DescriptorAllocator &allocator,
        venus::scene::Material::Ptr material) override {
    using namespace venus;

    VENUS_DECLARE_OR_RETURN_BAD_RESULT(
        scene::Material::Instance, instance,
        scene::Material::Instance::Config()
            .setWritePushConstants(
                VK_SHADER_STAGE_VERTEX_BIT,
                [](hermes::mem::Block &block,
                   const scene::PushConstantsContext &ctx) -> VeResult {
                  auto proj_view =
                      hermes::math::transpose((ctx.proj_view).matrix());
                  VENUS_RETURN_BAD_HE_RESULT(
                      block.resize(sizeof(hermes::geo::Transform)));
                  VENUS_RETURN_BAD_HE_RESULT(block.copy(&proj_view));
                  return VeResult::noError();
                })
            .setMaterial(material)
            .build(allocator))

    descriptor_writer_.clear();
    descriptor_writer_.writeBuffer(
        0, resources.data_buffer, sizeof(MAT_ScalarField::Data),
        resources.data_buffer_offset, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    descriptor_writer_.update(instance.localDescriptorSet(0));

    return Result<scene::Material::Instance>(std::move(instance));
  }

  Data data;
  Resources resources;
};

struct Sim {
  VeResult init() {
    using namespace venus;
    auto &gd = engine::GraphicsEngine::device();
    auto &cache = engine::GraphicsEngine::cache();
    auto &globals = venus::engine::GraphicsEngine::globals();

    VENUS_DECLARE_SHARED_PTR_FROM_RESULT_OR_RETURN_BAD_RESULT(
        scene::Material, mat, MAT_ScalarField::material(gd));
    material = mat;

    VENUS_DECLARE_SHARED_PTR_FROM_RESULT_OR_RETURN_BAD_RESULT(
        scene::AllocatedModel, triangle_model_ptr,
        scene::AllocatedModel::Config::fromShape(
            scene::shapes::triangle, //
            hermes::geo::point3(0.f, 0.f, 0.f),
            hermes::geo::point3(0.f, 1.f, 0.f),
            hermes::geo::point3(1.f, 0.f, 0.f), scene::shape_option_bits::none)
            .build(engine::GraphicsEngine::device()));
    model = triangle_model_ptr;

    f32 scalar_field[3] = {3.1f, 2.2f, 1.3f};

    VENUS_RETURN_BAD_RESULT(cache.buffers().addBuffer(
        "scalar_field",
        mem::AllocatedBuffer::Config::forStorage(
            sizeof(scalar_field), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
            .enableShaderDeviceAddress()
            .setDeviceLocal(),
        *gd));

    VENUS_RETURN_BAD_RESULT(cache.buffers().addBuffer(
        "ubo",
        mem::AllocatedBuffer::Config::forUniform(sizeof(MAT_ScalarField::Data)),
        *gd));

    VENUS_DECLARE_OR_RETURN_BAD_RESULT(
        auto, bf_offset_scalar_field,
        cache.buffers().allocate("scalar_field", sizeof(scalar_field)));

    VENUS_DECLARE_OR_RETURN_BAD_RESULT(
        auto, vf_offset_ubo,
        cache.buffers().allocate("ubo", sizeof(MAT_ScalarField::Data)));

    VENUS_DECLARE_OR_RETURN_BAD_RESULT(
        VkDeviceAddress, scalar_field_addr,
        cache.buffers().deviceAddress("scalar_field"));

    VENUS_DECLARE_OR_RETURN_BAD_RESULT(VkBuffer, vk_scalar_field,
                                       cache.buffers()["scalar_field"]);

    VENUS_DECLARE_OR_RETURN_BAD_RESULT(VkBuffer, vk_ubo,
                                       cache.buffers()["ubo"]);

    VENUS_RETURN_BAD_RESULT(
        pipeline::BufferWritter()
            .addBuffer(vk_scalar_field, scalar_field, sizeof(scalar_field))
            .immediateSubmit(gd));

    MAT_ScalarField parameters;
    parameters.data.mesh_buffer = model->vertexBufferAddress();
    parameters.data.scalar_buffer = scalar_field_addr;
    parameters.resources.data_buffer = vk_ubo;
    parameters.resources.data_buffer_offset = 0;

    VENUS_RETURN_BAD_RESULT(cache.buffers().copyBlock(
        "ubo", 0, &parameters.data, sizeof(MAT_ScalarField::Data)));

    VENUS_DECLARE_SHARED_PTR_FROM_RESULT_OR_RETURN_BAD_RESULT(
        venus::scene::Material::Instance, mat_instance,
        parameters.write(globals.descriptors.allocator(), material));
    model->setMaterialInstance(mat_instance);

    return VeResult::noError();
  }

  venus::scene::AllocatedModel::Ptr model;
  venus::scene::Material::Ptr material;
};

Sim sim;

VeResult init(venus::app::RA_SceneApp &app) {
  using namespace venus;

  VENUS_RETURN_BAD_RESULT(sim.init());

  VENUS_DECLARE_SHARED_PTR_FROM_RESULT_OR_RETURN_BAD_RESULT(
      scene::models::SkyBackground, sky,
      scene::models::SkyBackground::Config().build(
          engine::GraphicsEngine::device()));
  VENUS_DECLARE_SHARED_PTR_FROM_RESULT_OR_RETURN_BAD_RESULT(
      scene::models::CartesianGrid, grid,
      scene::models::CartesianGrid::Config().build(
          engine::GraphicsEngine::device()));

  app.scene().graph().addModel("sky", sky);
  app.scene().graph().addModel("sim", sim.model);
  app.scene().graph().addModel("grid", grid);

  return VeResult::noError();
}

int main() {
  VENUS_CHECK_VE_RESULT(venus::core::vk::init());
  venus::app::RA_SceneApp app;
  VENUS_ASSIGN_OR(
      app,
      venus::app::RA_SceneApp::Config()
          .setGraphicsEngineConfig(venus::engine::GraphicsEngine::Config()
                                       .setSynchronization2()
                                       .setDynamicRendering()
                                       .setShaderDemoteToHelperInvocation()
                                       .setBindless())
          .setDisplay<venus::io::GLFW_Window>("Hello Vulkan Display App",
                                              {1024, 1024})
          .setStartupFn(init)
          .setFPS(60)
          .setDurationInFrames(0)
          .build(),
      return -1);
  return app.run();
}
