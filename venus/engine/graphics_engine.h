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

/// \file   graphics_engine.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30
/// \brief  Venus Graphics GraphicsEngine

#pragma once

#include <venus/engine/gltf_io.h>
#include <venus/engine/graphics_device.h>
#include <venus/io/display.h>
#include <venus/pipeline/descriptors.h>
#include <venus/scene/material.h>
#include <venus/scene/texture.h>

#include <hermes/colors/color.h>
#include <hermes/geometry/transform.h>
#include <hermes/geometry/vector.h>

namespace venus::engine {

// *****************************************************************************
//                                                              GraphicsEngine
// *****************************************************************************

/// The venus engine holds all data and controls the workflow of the venus
/// program. The engine should be globally accessible, as it also offers
/// common data for all entities, such as descriptors, materials, etc.
/// \note static (singleton)
class GraphicsEngine {
public:
  // ***************************************************************************
  //                                                                   Globals
  // ***************************************************************************

  /// Holds globally all common resources for graphics, such as:
  ///   - Types
  ///   - Descriptors
  ///   - Defaults
  struct Globals {
    // *************************************************************************
    //                                                                   Types
    // *************************************************************************

    /// Types used throughout the application code.
    struct Types {
      /// Common push constants for shaders.
      struct DrawPushConstants {
        hermes::geo::Transform world_matrix;
        VkDeviceAddress vertex_buffer;
      };
      /// Common scene data for shaders.
      struct SceneData {
        hermes::geo::Transform view;
        hermes::geo::Transform proj;
        hermes::geo::Transform viewproj;
        hermes::colors::RGBA_Color ambient_color;
        hermes::geo::vec4 sunlight_direction; // w for sun power
        hermes::colors::RGBA_Color sunlight_color;
      };
    };

    /// \brief Set of venus built-in shaders.
    struct Shaders {
      pipeline::ShaderModule vert_mesh;
      pipeline::ShaderModule frag_mesh_pbr;

      // utils

      pipeline::ShaderModule vert_test;
      pipeline::ShaderModule vert_bindless_test;
      pipeline::ShaderModule frag_flat_color;

    private:
      friend struct Globals;
      VeResult init(VkDevice vk_device);
      void clear();
    };

    /// \brief Set of materials provided by the graphics engine.
    struct Materials {
      scene::Material gltf_metallic_roughness;
      scene::Material color;

    private:
      friend struct Globals;
      VeResult init(GraphicsDevice &graphics_device);
      void clear();
    };

    /// \brief Set of descriptors provided by the graphics engine.
    struct Descriptors {
      VkDescriptorSetLayout scene_data_layout{VK_NULL_HANDLE};

    private:
      friend struct Globals;
      VeResult init(GraphicsDevice &graphics_device);
      void clear();

      pipeline::DescriptorSet::Layout scene_data_layout_;
    };

    // *************************************************************************
    //                                                                Defaults
    // *************************************************************************

    /// \brief Set of default elements provided by the graphics engine.
    struct Defaults {

      mem::Image::Handle error_image;
      VkSampler linear_sampler{VK_NULL_HANDLE};
      VkSampler nearest_sampler{VK_NULL_HANDLE};

    private:
      friend struct Globals;
      VeResult init(engine::GraphicsDevice &graphics_device);
      void clear();

      mem::Image error_image_;
      scene::Sampler linear_sampler_;
      scene::Sampler nearest_sampler_;
    };

    /// Builds all resources.
    /// \param gd Graphics device.
    /// \return error status.
    VeResult init(engine::GraphicsDevice &gd);
    /// Release and clears all resources.
    VeResult cleanup();

    // Shaders
    Shaders shaders;
    // Materials
    Materials materials;
    // descriptors
    Descriptors descriptors;
    // defaults
    Defaults defaults;
  };

  // ***************************************************************************
  //                                                                    Cache
  // ***************************************************************************

  /// Holds resources that may be created/shared by multiple objects.
  struct Cache {
    mem::AllocatedBufferPool &allocatedBuffers();

    /// Release and clears all resources.
    VeResult cleanup();

  private:
    mem::AllocatedBufferPool allocated_buffer_pool_;
  };

  /// GraphicsEngine configuration.
  struct Config {
    Config &setSynchronization2();
    Config &setBindless();
    Config &setDynamicRendering();
    Config &setDeviceFeatures(const core::vk::DeviceFeatures &features);
    Config &setDeviceExtensions(const std::vector<std::string> &extensions);

    VeResult init(const io::Display *display) const;

  private:
    core::vk::DeviceFeatures device_features_;
    std::vector<std::string> device_extensions_;
  };

  struct FrameData {
    pipeline::DescriptorAllocator descriptor_allocator;
  };

  ~GraphicsEngine() = default;

  /// Initializes the engine.
  HERMES_NODISCARD static VeResult startup();
  /// Cleanup all resources and destroy all objects.
  /// \note this should be called before closing the program.
  HERMES_NODISCARD static VeResult shutdown();
  /// \return GraphicsEngine descriptors.
  HERMES_NODISCARD static Globals &globals();
  /// \return GraphicsEngine cache.
  HERMES_NODISCARD static Cache &cache();
  /// \return Graphics device.
  HERMES_NODISCARD static GraphicsDevice &device();

private:
  GraphicsEngine() noexcept = default;
  GraphicsEngine(const GraphicsEngine &) = delete;
  GraphicsEngine &operator=(const GraphicsEngine &) = delete;

  // static instance
  static GraphicsEngine s_instance;

  // data
  Globals globals_;
  // cache
  Cache cache_;

  // vulkan instance
  core::Instance instance_;
  // graphics device
  GraphicsDevice gd_;
  // output
  io::SurfaceKHR surface_;
};

} // namespace venus::engine
