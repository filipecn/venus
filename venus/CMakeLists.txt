set(VENUS_HEADERS
  core/debug.h
  core/instance.h
  core/physical_device.h
  core/vk_api.h
)
set(VENUS_SOURCES
  core/instance.cpp
  core/physical_device.cpp
  core/vk_api.cpp
)

add_library(venus STATIC ${VENUS_HEADERS} ${VENUS_SOURCES})

# target_include_directories(venus_engine PRIVATE "${VENUS_SOURCE_DIR}/src")
target_include_directories(
  venus PUBLIC "$<BUILD_INTERFACE:${VENUS_SOURCE_DIR}>"
                    "$<INSTALL_INTERFACE:${VENUS_SOURCE_DIR}>")

target_link_libraries(
  venus PUBLIC optimized hermes glfw volk GPUOpen::VulkanMemoryAllocator)
#debug
#                   gcov)

target_compile_definitions(venus PUBLIC VENUS_INCLUDE_TO_STRING)

set_target_properties(
  venus
  PROPERTIES CXX_STANDART 23
             CXX_STANDARD_REQUIRED YES
             LINKER_LANGUAGE CXX
             OUTPUT_NAME "venus"
             FOLDER "VENUS")
# CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-format-style='file';-fix;-p;")

if(CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(
    venus
    PUBLIC
      "$<$<CONFIG:Debug>:-g;-pg;-Wall;-Werror;-Wextra;-O0;-fprofile-arcs;-ftest-coverage;--coverage;-fPIC>"
      "$<$<CONFIG:Release>:-O3;-fPIC;>")
endif(CMAKE_COMPILER_IS_GNUCXX)
