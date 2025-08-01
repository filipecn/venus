include(FetchContent)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.4
  GIT_SHALLOW TRUE
  GIT_PROGRESS TRUE
)

set(GLFW_BUILD_DOCS
    OFF
    CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS
    OFF
    CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES
    OFF
    CACHE BOOL "" FORCE)
set(GLFW_BUILD_X11
    ON
    CACHE BOOL "" FORCE)
set(GLFW_BUILD_WAYLAND
    OFF
    CACHE BOOL "" FORCE)
  set(GLFW_BUILD_COCOA
    OFF
    CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw)
