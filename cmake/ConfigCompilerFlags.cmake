set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_VERBOSE_MAKEFILE ON)
# ##############################################################################
# CHECKS                                                                       #
# ##############################################################################
include(CheckCXXCompilerFlag)

check_cxx_compiler_flag(-std=c++23 COMPILER_SUPPORTS_CXX23)
#if(NOT COMPILER_SUPPORTS_CXX23)
#  message(FATAL_ERROR "c++23 support required!")
#endif(NOT COMPILER_SUPPORTS_CXX23)

# endianess
if(CMAKE_CXX_BYTE_ORDER)
  if(CMAKE_CXX_BYTE_ORDER STREQUAL "BIG_ENDIAN")
    set(ARCH_IS_BIG_ENDIAN_TARGET 1)
  else()
    set(ARCH_IS_BIG_ENDIAN_TARGET 0)
  endif()
else()
  include(TestBigEndian)
  test_big_endian(BIG_ENDIAN)
  set(ARCH_IS_BIG_ENDIAN_TARGET ${BIG_ENDIAN})
endif()

# ##############################################################################
# CXX variables / compiler flags #
# ##############################################################################
set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE "ON")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CONFIGURATION_TYPES Debug Release)
