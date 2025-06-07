include(FetchContent)

FetchContent_Declare(
  volk
  GIT_REPOSITORY https://github.com/zeux/volk.git
  GIT_TAG 1.3.270)

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
elseif(CMAKE_SYSTEM_NAME STREQUAL Darwin)
  set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_MACOS_MVK)
endif()

FetchContent_MakeAvailable(volk)
