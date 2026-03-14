include(FetchContent)

# 1. Declare the dependency
FetchContent_Declare(
    tinyobjloader
    GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
    GIT_TAG        v1.0.6 
)

# 2. Make the dependency available
FetchContent_MakeAvailable(tinyobjloader)
