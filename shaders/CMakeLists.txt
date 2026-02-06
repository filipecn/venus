find_program(GLSL_VALIDATOR glslangValidator 
  HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/
  REQUIRED
)

find_program(SLANGC_EXECUTABLE slangc
    PATHS "$ENV{VULKAN_SDK}/bin"
    REQUIRED
)

file(GLOB_RECURSE GLSL_SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/*.frag"
  "${CMAKE_CURRENT_LIST_DIR}/*.vert"
  "${CMAKE_CURRENT_LIST_DIR}/*.comp"
  "${CMAKE_CURRENT_LIST_DIR}/*.rgen"
  "${CMAKE_CURRENT_LIST_DIR}/*.rmiss"
  "${CMAKE_CURRENT_LIST_DIR}/*.rchit"
    )

file(GLOB_RECURSE SLANG_SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/*.slang"
    )

foreach(GLSL ${GLSL_SOURCE_FILES})
  message(STATUS "BUILDING SHADER")
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${CMAKE_CURRENT_LIST_DIR}/${FILE_NAME}.spv")
  message(STATUS ${GLSL})
  message(STATUS COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV} --target-env vulkan1.4)
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV} --target-env vulkan1.4
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)


foreach(SLANG_FILE ${SLANG_SOURCE_FILES})
    # Get the filename without extension for the output name
    get_filename_component(SHADER_NAME "${SLANG_FILE}" NAME_WE)
    # Define the output path for the SPIR-V file
    set(OUTPUT_SPV_FILE "${CMAKE_CURRENT_BINARY_DIR}/shaders/${SHADER_NAME}.spv")

    # Add a custom command to compile the shader
    add_custom_command(
        OUTPUT "${OUTPUT_SPV_FILE}"
        COMMAND "${SLANGC_EXECUTABLE}"
        ARGS "${CMAKE_CURRENT_SOURCE_DIR}/${SLANG_FILE}" -o "${OUTPUT_SPV_FILE}" --target spirv 
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${SLANG_FILE}"
        COMMENT "Compiling Slang shader ${SLANG_FILE} to SPIR-V"
        # Optional: Add include directories if your shaders use #include
        # ARGS -I "${CMAKE_CURRENT_SOURCE_DIR}/include" ...
    )

    # Add the generated file to a list
  list(APPEND SPIRV_BINARY_FILES "${OUTPUT_SPV_FILE}")
endforeach(SLANG_FILE)

add_custom_target(
    venus_shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
    )
