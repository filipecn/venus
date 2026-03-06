function(build_example EXAMPLE_NAME)

add_executable(${EXAMPLE_NAME} ${EXAMPLE_NAME}.cpp)

set_target_properties(
  ${EXAMPLE_NAME}
  PROPERTIES CXX_STANDART 23
             CXX_STANDARD_REQUIRED YES
             CXX_EXTENSIONS YES)

target_compile_definitions(${EXAMPLE_NAME} PRIVATE 
  VENUS_EXAMPLE_ASSETS_PATH="${VENUS_EXAMPLES_DIR}/assets")

target_link_libraries(${EXAMPLE_NAME} PRIVATE venus)

target_compile_definitions(${EXAMPLE_NAME} PRIVATE 
  VENUS_SHADERS_PATH="${VENUS_SOURCE_DIR}/shaders")

if(CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(
    ${EXAMPLE_NAME}
    PUBLIC
      "$<$<CONFIG:Debug>:-g;-pg;-Wall;-Werror;-Wextra;-O0;-fprofile-arcs;-ftest-coverage;--coverage;-fPIC>"
      "$<$<CONFIG:Release>:-O3;-fPIC;>")
endif(CMAKE_COMPILER_IS_GNUCXX)

endfunction(build_example)



build_example(hello_ra)
build_example(sim_surface_field)

