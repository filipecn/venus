FetchContent_Declare(
        glslang
        GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
        GIT_TAG 15.3.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)

set(PROJECT_IS_TOP_LEVEL OFF)
set(ALLOW_EXTERNAL_SPIRV_TOOLS OFF)
set(ENABLE_OPT OFF)

FetchContent_GetProperties(glslang)
FetchContent_MakeAvailable(glslang)


