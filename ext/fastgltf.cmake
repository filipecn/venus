FetchContent_Declare(
        fastgltf
        GIT_REPOSITORY https://github.com/spnda/fastgltf.git
        GIT_TAG v0.8.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)

FetchContent_GetProperties(fastgltf)
FetchContent_MakeAvailable(fastgltf)

