# ##############################################################################
# CMAKE                                                                        #
# ##############################################################################
cmake_minimum_required(VERSION 3.12..3.30 FATAL_ERROR)

project(
  VENUS
  VERSION 0.01
  DESCRIPTION "Real-time graphics framework"
  LANGUAGES C CXX)

set(VENUS_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR})

# cmake print functions
include(CMakePrintHelpers)

# ##############################################################################
# OPTIONS                                                                      #
# ##############################################################################
option(BUILD_ALL "build all targets" OFF)

# ##############################################################################
# COMPILATION                                                                  #
# ##############################################################################
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
include(ConfigCompilerFlags)
include(ExternalPrograms)

# ##############################################################################
# DEPENDENCIES                                                                 #
# ##############################################################################
include(ExternalProject)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/ext")
include(catch2)
include(glfw)
include(vma)
include(volk)

# ##############################################################################
# ENGINE                                                                       #
# ##############################################################################
add_subdirectory(venus)

# ##############################################################################
# TESTS                                                                        #
# ##############################################################################
# add_subdirectory(tests)

# ##############################################################################
# COMPILATION/RUN SANITY TEST                                                  #
# ##############################################################################
# try_compile(COMPILATION_TEST_RESULT bindir SOURCES source.cpp)
# try_run(RUN_TEST_RESULT bindr SOURCES source.cpp)

add_subdirectory(examples)
