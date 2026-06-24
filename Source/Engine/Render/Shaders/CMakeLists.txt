function(compile_shaders TARGET)
    if(WIN32 AND DEFINED ENV{VULKAN_SDK})
        find_program(
                GLSLC
                NAMES glslc
                HINTS "$ENV{VULKAN_SDK}/Bin"
                REQUIRED
        )
    else()
        find_program(
                GLSLC
                NAMES glslc
                REQUIRED
        )
    endif()

    set(SPV_FILES)

    set(SHADER_OUTPUT_DIRECTORY "${GAME_OUTPUT_DIRECTORY}/Shaders")
    
    foreach(SHADER ${ARGN})
        get_filename_component(FILE_NAME ${SHADER} NAME)

        set(SPV_FILE
                ${SHADER_OUTPUT_DIRECTORY}/${FILE_NAME}.spv
        )

        add_custom_command(
                OUTPUT ${SPV_FILE}
                COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_OUTPUT_DIRECTORY}
                COMMAND ${GLSLC} ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER} -o ${SPV_FILE}
                DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER}"
                COMMENT "Compiling shader ${FILE_NAME}"
                VERBATIM
        )

        list(APPEND SPV_FILES ${SPV_FILE})
    endforeach()

    set_source_files_properties(
            ${SPV_FILES}
            PROPERTIES GENERATED TRUE
    )
    
    add_custom_target(
            ${TARGET}_shaders
            DEPENDS ${SPV_FILES}
            SOURCES ${ARGN}
    )

    add_dependencies(Game Game_shaders)
endfunction()

compile_shaders(
        Game
        Shader.vert
        Shader.frag
        LineShader.vert
        LineShader.frag
)