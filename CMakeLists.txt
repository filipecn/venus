# ##############################################################################
# CMAKE                                                                        #
# ##############################################################################
cmake_minimum_required(VERSION 3.12..3.30 FATAL_ERROR)

project(
  VENUS
  VERSION 0.01
  DESCRIPTION "Real-time graphics engine framework"
  LANGUAGES C CXX)

cmake_path(SET VENUS_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR})

# cmake print functions
include(CMakePrintHelpers)

# ##############################################################################
# OPTIONS                                                                      #
# ##############################################################################
option(VENUS_BUILD_ALL "build all targets" OFF)
option(VENUS_BUILD_TESTS "build tests" OFF)
option(VENUS_BUILD_EXAMPLES "build examples" OFF)

# ##############################################################################
# COMPILATION                                                                  #
# ##############################################################################
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
include(ConfigCompilerFlags)
include(ExternalPrograms)

# ##############################################################################
# DEPENDENCIES                                                                 #
# ##############################################################################
add_subdirectory(ext)

# ##############################################################################
# ENGINE                                                                       #
# ##############################################################################
add_subdirectory(shaders)
add_subdirectory(venus)

# ##############################################################################
# TESTS                                                                        #
# ##############################################################################
#if (VENUS_BUILD_TESTS)
  # add_subdirectory(tests)
  #endif (VENUS_BUILD_TESTS)

# ##############################################################################
# COMPILATION/RUN SANITY TEST                                                  #
# ##############################################################################
# try_compile(COMPILATION_TEST_RESULT bindir SOURCES source.cpp)
# try_run(RUN_TEST_RESULT bindr SOURCES source.cpp)

if (VENUS_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif (VENUS_BUILD_EXAMPLES)
