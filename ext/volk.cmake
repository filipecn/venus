include(FetchContent)

# Locate the platform's Vulkan headers and loader through CMake rather than
# constructing an SDK include path in the configure preset.
find_package(Vulkan REQUIRED)

message(STATUS "Vulkan include directories: ${Vulkan_INCLUDE_DIRS}")
message(STATUS "Vulkan loader library: ${Vulkan_LIBRARY}")
foreach(VULKAN_INCLUDE_DIR IN LISTS Vulkan_INCLUDE_DIRS)
  set(VULKAN_PLATFORM_HEADER "${VULKAN_INCLUDE_DIR}/vulkan/vk_platform.h")
  if(EXISTS "${VULKAN_PLATFORM_HEADER}")
    message(STATUS "Vulkan platform header found: ${VULKAN_PLATFORM_HEADER}")
  else()
    message(STATUS "Vulkan platform header missing: ${VULKAN_PLATFORM_HEADER}")
  endif()
endforeach()

FetchContent_Declare(
  volk
  GIT_REPOSITORY https://github.com/zeux/volk.git
  GIT_TAG 1.4.350)

set(VOLK_INSTALL
  ON
  CACHE BOOL "" FORCE)
set(VOLK_PULL_IN_VULKAN
  ON
  CACHE BOOL "" FORCE)
set(VMA_BUILD_SAMPLES
  OFF
  CACHE BOOL "" FORCE)

# Set a suitable platform define to compile volk with.
if(CMAKE_SYSTEM_NAME STREQUAL Windows)
  set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)
  set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_XLIB_KHR)
  set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_XCB_KHR)
elseif(CMAKE_SYSTEM_NAME STREQUAL Darwin)
  set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_MACOS_MVK)
  set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_METAL_EXT)
endif()

FetchContent_MakeAvailable(volk)

target_include_directories(volk PUBLIC ${Vulkan_INCLUDE_DIRS})
target_include_directories(volk_headers INTERFACE ${Vulkan_INCLUDE_DIRS})

set(VOLK_INCLUDE_DIR ${volk_SOURCE_DIR} CACHE STRING "" FORCE)
set(VOLK_DEFINES ${VOLK_STATIC_DEFINES} CACHE STRING "" FORCE)



