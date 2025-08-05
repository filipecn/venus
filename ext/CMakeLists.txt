include(ExternalProject)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
include(glfw)
include(vma)
include(volk)
include(hermes)
include(glslang)

if (BUILD_TESTS)  
  include(catch2)
endif (BUILD_TESTS)

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
