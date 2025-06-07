include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.6.0)

set(CATCH_INSTALL_DOCS
    OFF
    CACHE BOOL "" FORCE)

set(CATCH_INSTALL_EXTRAS
    ON
    CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(Catch2)
