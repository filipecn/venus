include(ExternalProject)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

set(DEPS_INCLUDE_DIRS "")

add_library(DEPS INTERFACE)

if(VENUS_INCLUDE_VDB)
  include(openvdb)
  target_link_libraries(DEPS INTERFACE openvdb)
endif (VENUS_INCLUDE_VDB)

if(VENUS_INCLUDE_GLTF)
  include(fastgltf)
  target_link_libraries(DEPS INTERFACE fastgltf)
  list(APPEND DEPS_INCLUDE_DIRS ";${FASTGLTF_INCLUDE_DIR}")
endif (VENUS_INCLUDE_GLTF)

include(glfw)
include(hermes)
include(stb)
include(imgui)

list(APPEND DEPS_INCLUDE_DIRS ";${HERMES_INCLUDE_DIR}")
list(APPEND DEPS_INCLUDE_DIRS ";${STB_INCLUDE_DIR}")
list(APPEND DEPS_INCLUDE_DIRS ";${IMGUI_INCLUDE_DIR}")

target_link_libraries(DEPS INTERFACE 
  glfw
  hermes
  imgui
)

include(vma)
include(volk)
include(glslang)

list(APPEND DEPS_INCLUDE_DIRS ";${VMA_INCLUDE_DIR}")
list(APPEND DEPS_INCLUDE_DIRS ";${VOLK_INCLUDE_DIR}")
list(APPEND DEPS_INCLUDE_DIRS ";${GLSLANG_INCLUDE_DIR}")

add_library(VK INTERFACE)

target_link_libraries(VK INTERFACE
  volk
  volk_headers
  GPUOpen::VulkanMemoryAllocator
  #Vulkan::SafeStruct
  #Vulkan::LayerSettings
  #Vulkan::UtilityHeaders
  #Vulkan::CompilerConfiguration
  glslang
  MachineIndependent 
  glslang-default-resource-limits
)

target_link_libraries(DEPS INTERFACE VK)

if (VENUS_BUILD_TESTS)  
  include(catch2)
endif (VENUS_BUILD_TESTS)

set(DEPS_INCLUDE_DIRS ${DEPS_INCLUDE_DIRS} CACHE STRING "" FORCE)
