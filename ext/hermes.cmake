include(FetchContent)

FetchContent_Declare(
  hermes
  GIT_REPOSITORY https://github.com/filipecn/hermes.git
  GIT_TAG "origin/modern"
)

FetchContent_MakeAvailable(hermes)

SET(HERMES_INCLUDES ${hermes_SOURCE_DIR})
