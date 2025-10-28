include(FetchContent)

FetchContent_Declare(
  vma
  GIT_REPOSITORY
    https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
  GIT_TAG v3.1.0)

set(VMA_BUILD_DOCUMENTATION
    OFF
    CACHE BOOL "" FORCE)
set(VMA_ENABLE_INSTALL
    ON
    CACHE BOOL "" FORCE)
set(VMA_BUILD_SAMPLES
    OFF
    CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(vma)

set(VMA_INCLUDE_DIR ${vma_SOURCE_DIR}/include CACHE STRING "" FORCE)

