add_executable(main main.cpp)

set_target_properties(
  main
  PROPERTIES CXX_STANDART 23
             CXX_STANDARD_REQUIRED YES
             CXX_EXTENSIONS YES)
# CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-format-style='file';-fix;-p")

target_compile_definitions(main PRIVATE 
  VENUS_EXAMPLE_ASSETS_PATH="${VENUS_EXAMPLES_DIR}/assets")

target_link_libraries(main PRIVATE venus)
# glfw volk GPUOpen::VulkanMemoryAllocator) std=c++20 stdc++
