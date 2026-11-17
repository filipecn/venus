add_library(venus STATIC
  app/display_app.cpp
  app/scene.cpp
  app/scene_app.cpp

  core/device.cpp
  core/device_queue.cpp
  core/instance.cpp
  core/physical_device.cpp
  core/sync.cpp
  core/vk_api.cpp

  engine/frame_loop.cpp
  engine/graphics_device.cpp
  engine/graphics_engine.cpp
  engine/shapes.cpp

  io/glfw_display.cpp
  io/surface.cpp
  io/swapchain.cpp

  mem/buffer.cpp
  mem/device_memory.cpp
  mem/image.cpp
  mem/layout.cpp

  pipeline/command_buffer.cpp
  pipeline/descriptors.cpp
  pipeline/framebuffer.cpp
  pipeline/pipeline.cpp
  pipeline/rasterizer.cpp
  pipeline/ray_tracer.cpp
  pipeline/renderpass.cpp
  pipeline/shader_module.cpp

  scene/models/cartesian_grid.cpp
  scene/models/sky_background.cpp
  scene/acceleration_structure.cpp
  scene/camera.cpp
  scene/material.cpp
  scene/materials.cpp
  scene/model.cpp
  scene/scene_graph.cpp
  scene/texture.cpp

  ui/camera.cpp
  ui/picking.cpp
)

target_compile_definitions(venus PRIVATE 
  VENUS_SHADERS_PATH="${VENUS_SOURCE_DIR}/shaders")

target_include_directories(
  venus PUBLIC "$<BUILD_INTERFACE:${VENUS_SOURCE_DIR};${VENUS_DEPS_INCLUDE_DIRS}>"
  "$<INSTALL_INTERFACE:${VENUS_SOURCE_DIR}>")

add_dependencies(venus venus_shaders VENUS_DEPS)

target_link_libraries(venus PUBLIC optimized VENUS_DEPS debug DEPS)

include(CMakePrintHelpers)
cmake_print_properties(TARGETS venus PROPERTIES INTERFACE_INCLUDE_DIRECTORIES)

target_compile_definitions(venus PUBLIC VOLK_DEFINES)

if(MSVC)
target_compile_definitions(venus PUBLIC NOMINMAX)
endif(MSVC)

if(VENUS_INCLUDE_DEBUG_TRAITS)
  target_compile_definitions(venus PUBLIC VENUS_INCLUDE_DEBUG_TRAITS)
  target_compile_definitions(venus PUBLIC HERMES_INCLUDE_DEBUG_TRAITS)
endif(VENUS_INCLUDE_DEBUG_TRAITS)

if (VENUS_INCLUDE_VDB)
  target_compile_definitions(venus PUBLIC VENUS_INCLUDE_VDB)
endif (VENUS_INCLUDE_VDB)

if (VENUS_INCLUDE_GLTF)
  target_compile_definitions(venus PUBLIC VENUS_INCLUDE_GLTF)
endif (VENUS_INCLUDE_GLTF)


set_target_properties(
  venus
  PROPERTIES CXX_STANDART 23
  CXX_STANDARD_REQUIRED YES
  LINKER_LANGUAGE CXX
  OUTPUT_NAME "venus"
  FOLDER "VENUS")
# CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-format-style='file';-fix;-p;")

if(CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(
    venus
    PUBLIC 
    "$<$<CONFIG:Debug>:-g;-pg;-Wall;-Wextra;-Werror;-O0;-fprofile-arcs;-ftest-coverage;--coverage;-fPIC>"
    "$<$<CONFIG:Release>:-O3;-fPIC;>")
elseif(MSVC)
  target_compile_options(
    venus
    PUBLIC 
    "$<$<CONFIG:Debug>:/Zc:__cplusplus;/Zc:preprocessor;/EHsc;/MDd>"
    "$<$<CONFIG:Release>:/Zc:__cplusplus;/Zc:preprocessor/EHsc;/MD>")
endif(CMAKE_COMPILER_IS_GNUCXX)

