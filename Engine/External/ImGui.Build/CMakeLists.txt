set(IMGUI_DIR ${CMAKE_CURRENT_LIST_DIR}/../ImGui)

add_library(ImGui STATIC
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp

        ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp

        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
)

set_target_properties(ImGui PROPERTIES
        POSITION_INDEPENDENT_CODE ON
)

target_include_directories(ImGui PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
        ${IMGUI_DIR}/misc/cpp
)

target_link_libraries(ImGui PUBLIC
        glfw
        Vulkan::Vulkan
)