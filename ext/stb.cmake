include(FetchContent)

FetchContent_Declare(
  stb
  GIT_REPOSITORY
    https://github.com/nothings/stb.git
  GIT_TAG master)

FetchContent_MakeAvailable(stb)

set(STB_INCLUDE_DIR ${stb_SOURCE_DIR} CACHE STRING "" FORCE)


