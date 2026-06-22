cmake_minimum_required(VERSION 4.2.2)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
        "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")

project(
        EcsGameEngine
        VERSION 1.0
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(Vulkan_LIBRARY "/usr/lib64/libvulkan.so.1")

find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

# ----------------------------------------------------------
# Engine
# ----------------------------------------------------------

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
        GameEngine/*.cpp
)

file(GLOB_RECURSE ENGINE_MODULES CONFIGURE_DEPENDS
        GameEngine/*.cppm
)

add_library(Engine SHARED)

target_sources(Engine
        PRIVATE
        ${ENGINE_SOURCES}

        PUBLIC
        FILE_SET CXX_MODULES FILES
        ${ENGINE_MODULES}
)

target_include_directories(Engine
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/GameEngine
)

target_link_libraries(Engine
        PUBLIC
        Vulkan::Vulkan
        glfw
        imgui::imgui
)

target_compile_features(Engine PUBLIC cxx_std_23)

set_property(TARGET Engine PROPERTY CXX_MODULE_STD ON)

# ----------------------------------------------------------
# Game
# ----------------------------------------------------------

add_executable(Game
        Game/Game.cpp
)

target_link_libraries(Game
        PRIVATE
        Engine
)

target_compile_features(Game PUBLIC cxx_std_23)
set_property(TARGET Game PROPERTY CXX_MODULE_STD ON)
