include(FetchContent)

FetchContent_Declare(
  imgui_src
  GIT_REPOSITORY
    https://github.com/ocornut/imgui.git
  GIT_TAG v1.92.4)

FetchContent_MakeAvailable(imgui_src)

set(IMGUI_INCLUDE_DIR ${imgui_src_SOURCE_DIR} CACHE STRING "" FORCE)

add_library(imgui STATIC)

target_include_directories(imgui PUBLIC ${IMGUI_INCLUDE_DIR})

target_sources(imgui PRIVATE 
  ${IMGUI_INCLUDE_DIR}/imgui.h
  ${IMGUI_INCLUDE_DIR}/imgui.cpp
  ${IMGUI_INCLUDE_DIR}/imgui_demo.cpp
  ${IMGUI_INCLUDE_DIR}/imgui_draw.cpp
  ${IMGUI_INCLUDE_DIR}/imgui_widgets.cpp
  ${IMGUI_INCLUDE_DIR}/imgui_tables.cpp
  ${IMGUI_INCLUDE_DIR}/backends/imgui_impl_vulkan.cpp
  ${IMGUI_INCLUDE_DIR}/backends/imgui_impl_glfw.cpp
  )

target_compile_definitions(imgui PUBLIC IMGUI_IMPL_VULKAN_NO_PROTOTYPES)
target_link_libraries(imgui PUBLIC VK)
