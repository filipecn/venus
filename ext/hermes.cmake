include(FetchContent)

FetchContent_Declare(
  hermes 
  GIT_REPOSITORY https://github.com/filipecn/hermes.git
  GIT_TAG modern)

set(HERMES_INCLUDE_TO_STRING ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(hermes)

set(HERMES_INCLUDE_DIR ${hermes_SOURCE_DIR} CACHE STRING "" FORCE)
