set(VENUS_HEADERS
  app/display_app.h
  app/graphics_device.h

  core/device.h
  core/instance.h
  core/physical_device.h
  core/sync.h
  core/vk_api.h

  io/display.h
  io/glfw_display.h
  io/swapchain.h

  mem/buffer.h
  mem/device_memory.h
  mem/image.h
  mem/layout.h

  pipeline/command_buffer.h
  pipeline/descriptors.h
  pipeline/framebuffer.h
  pipeline/pipeline.h
  pipeline/renderpass.h
  pipeline/shader_module.h
  
  utils/debug.h
  utils/macros.h
  utils/result.h
  utils/vk_debug.h
)
set(VENUS_SOURCES
  app/display_app.cpp
  app/graphics_device.cpp
  
  core/device.cpp
  core/instance.cpp
  core/physical_device.cpp
  core/sync.cpp
  core/vk_api.cpp
  
  io/display.cpp
  io/glfw_display.cpp
  io/swapchain.cpp

  mem/buffer.cpp
  mem/device_memory.cpp
  mem/image.cpp
  mem/layout.cpp

  pipeline/command_buffer.cpp
  pipeline/descriptors.cpp
  pipeline/framebuffer.cpp
  pipeline/pipeline.cpp
  pipeline/renderpass.cpp
  pipeline/shader_module.cpp
)

add_library(venus STATIC ${VENUS_HEADERS} ${VENUS_SOURCES})

# target_include_directories(venus_engine PRIVATE "${VENUS_SOURCE_DIR}/src")
target_include_directories(
  venus PUBLIC "$<BUILD_INTERFACE:${VENUS_SOURCE_DIR}>"
                    "$<INSTALL_INTERFACE:${VENUS_SOURCE_DIR}>")

target_link_libraries( venus PUBLIC optimized hermes glfw VK)
#debug
#                   gcov)

target_compile_definitions(venus PUBLIC VENUS_INCLUDE_TO_STRING)

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
      "$<$<CONFIG:Debug>:-g;-pg;-Wall;-Werror;-Wextra;-O0;-fprofile-arcs;-ftest-coverage;--coverage;-fPIC>"
      "$<$<CONFIG:Release>:-O3;-fPIC;>")
endif(CMAKE_COMPILER_IS_GNUCXX)
