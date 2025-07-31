include(FetchContent)

FetchContent_Declare(
  vul
  GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Utility-Libraries.git
  GIT_TAG main)

set(BUILD_TESTS
  OFF
  CACHE BOOL "" FORCE)

set(UPDATE_DEPS
  ON
  CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(vul)
