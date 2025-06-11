set(VENUS_ENGINE_HEADERS
  #core/debug.h
    #core/instance.h
    #core/physical_device.h
    core/vk.h
    #io/graphics_display.h
    #io/surface_khr.h
  )
set(VENUS_ENGINE_SOURCES
  #core/instance.cpp
  #core/physical_device.cpp
    core/vk.cpp
    #io/graphics_display.cpp
    #io/surface_khr.cpp
  )

add_library(venus_engine STATIC ${VENUS_ENGINE_HEADERS} ${VENUS_ENGINE_SOURCES})

# target_include_directories(venus_engine PRIVATE "${VENUS_SOURCE_DIR}/src")
target_include_directories(
  venus_engine PUBLIC "$<BUILD_INTERFACE:${VENUS_SOURCE_DIR}>"
                    "$<INSTALL_INTERFACE:${VENUS_SOURCE_DIR}>")

target_link_libraries(
  venus_engine PUBLIC optimized hermes glfw volk GPUOpen::VulkanMemoryAllocator)
#debug
#                   gcov)

set_target_properties(
  venus_engine
  PROPERTIES CXX_STANDART 23
             CXX_STANDARD_REQUIRED YES
             LINKER_LANGUAGE CXX
             OUTPUT_NAME "venus_engine"
             FOLDER "VENUS_ENGINE")
# CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-format-style='file';-fix;-p;")

if(CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(
    venus_engine
    PUBLIC
      "$<$<CONFIG:Debug>:-g;-pg;-Wall;-Werror;-Wextra;-O0;-fprofile-arcs;-ftest-coverage;--coverage;-fPIC>"
      "$<$<CONFIG:Release>:-O3;-fPIC;>")
endif(CMAKE_COMPILER_IS_GNUCXX)
