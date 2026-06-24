function(compile_shaders TARGET)
    find_program(GLSLC glslc REQUIRED)

    set(SPV_FILES)

    foreach(SHADER ${ARGN})
        get_filename_component(FILE_NAME ${SHADER} NAME)

        set(SPV_FILE
                ${CMAKE_BINARY_DIR}/Shaders/${FILE_NAME}.spv
        )

        add_custom_command(
                OUTPUT ${SPV_FILE}
                COMMAND ${CMAKE_COMMAND} -E make_directory
                ${CMAKE_BINARY_DIR}/Shaders
                COMMAND ${GLSLC}
                ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER}
                -o ${SPV_FILE}
                DEPENDS ${SHADER}
        )

        list(APPEND SPV_FILES ${SPV_FILE})
    endforeach()

    add_custom_target(
            ${TARGET}_shaders
            ALL
            DEPENDS ${SPV_FILES}
    )
endfunction()

compile_shaders(
        Game
        Shader.vert
        Shader.frag
        LineShader.vert
        LineShader.frag
)