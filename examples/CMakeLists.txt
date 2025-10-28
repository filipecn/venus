add_executable(main main.cpp)

set_target_properties(
  main
  PROPERTIES CXX_STANDART 23
             CXX_STANDARD_REQUIRED YES
             CXX_EXTENSIONS YES)

target_compile_definitions(main PRIVATE 
  VENUS_EXAMPLE_ASSETS_PATH="${VENUS_EXAMPLES_DIR}/assets")

target_link_libraries(main PRIVATE venus)

if(CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(
    main
    PUBLIC
      "$<$<CONFIG:Debug>:-g;-pg;-Wall;-Werror;-Wextra;-O0;-fprofile-arcs;-ftest-coverage;--coverage;-fPIC>"
      "$<$<CONFIG:Release>:-O3;-fPIC;>")
endif(CMAKE_COMPILER_IS_GNUCXX)
