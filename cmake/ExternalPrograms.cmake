# ##############################################################################
# COMPILATION CACHE                                                            #
# ##############################################################################
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

# ##############################################################################
# CLANG TIDY                                                                   #
# ##############################################################################
# find_program(CLANG_TIDY_EXE NAMES "clang-tidy") if(CLANG_TIDY_EXE)
# set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}") endif()
