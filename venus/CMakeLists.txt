set(VENUS_HEADERS
  app/display_app.h
  app/scene.h
  app/scene_app.h

  core/device.h
  core/instance.h
  core/physical_device.h
  core/sync.h
  core/vk_api.h

  engine/frame_loop.h
  engine/graphics_device.h
  engine/graphics_engine.h
  engine/renderers.h
  engine/shapes.h

  io/display.h
  io/glfw_display.h
  io/surface.h
  io/swapchain.h

  mem/buffer.h
  mem/device_memory.h
  mem/image.h
  mem/layout.h

  pipeline/command_buffer.h
  pipeline/descriptors.h
  pipeline/framebuffer.h
  pipeline/pipeline.h
  pipeline/rasterizer.h
  pipeline/ray_tracer.h
  pipeline/renderpass.h
  pipeline/shader_module.h

  scene/acceleration_structure.h
  scene/camera.h
  scene/material.h
  scene/materials.h
  scene/model.h
  scene/scene_graph.h
  scene/texture.h

  ui/camera.h

  utils/debug.h
  utils/macros.h
  utils/result.h
  utils/vk_debug.h
)
set(VENUS_SOURCES
  app/display_app.cpp
  app/scene.cpp
  app/scene_app.cpp

  core/device.cpp
  core/instance.cpp
  core/physical_device.cpp
  core/sync.cpp
  core/vk_api.cpp

  engine/frame_loop.cpp
  engine/graphics_device.cpp
  engine/graphics_engine.cpp
  engine/renderers.cpp
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

  scene/acceleration_structure.cpp
  scene/camera.cpp
  scene/material.cpp
  scene/materials.cpp
  scene/model.cpp
  scene/scene_graph.cpp
  scene/texture.cpp

  ui/camera.cpp
)

add_library(venus STATIC ${VENUS_HEADERS} ${VENUS_SOURCES})

target_compile_definitions(venus PRIVATE 
  VENUS_SHADERS_PATH="${VENUS_SOURCE_DIR}/shaders")

target_include_directories(
  venus PUBLIC "$<BUILD_INTERFACE:${VENUS_SOURCE_DIR};${DEPS_INCLUDE_DIRS}>"
  "$<INSTALL_INTERFACE:${VENUS_SOURCE_DIR}>")

add_dependencies(venus venus_shaders DEPS)

target_link_libraries(venus PUBLIC optimized DEPS debug DEPS gcov)

include(CMakePrintHelpers)
cmake_print_properties(TARGETS venus PROPERTIES INTERFACE_INCLUDE_DIRECTORIES)

target_compile_definitions(venus PUBLIC VOLK_DEFINES)

if(VENUS_INCLUDE_TO_STRING_METHODS)
  target_compile_definitions(venus PUBLIC VENUS_INCLUDE_TO_STRING)
  target_compile_definitions(venus PUBLIC HERMES_INCLUDE_TO_STRING)
endif(VENUS_INCLUDE_TO_STRING_METHODS)

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
endif(CMAKE_COMPILER_IS_GNUCXX)

