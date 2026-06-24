cmake_minimum_required(VERSION 4.2.2)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

if(ENABLE_ASAN)
    add_compile_options(
            -fsanitize=address
            -fno-omit-frame-pointer
    )

    add_link_options(
            -fsanitize=address
    )
endif()

set(CMAKE_CONFIGURATION_TYPES "Debug;Development;Release" CACHE STRING "" FORCE)

set(CMAKE_CXX_FLAGS_DEVELOPMENT "-Og -g")
set(CMAKE_EXE_LINKER_FLAGS_DEVELOPMENT "")
set(CMAKE_SHARED_LINKER_FLAGS_DEVELOPMENT "")

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "")

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")

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

message(STATUS "Vulkan_LIBRARY = ${Vulkan_LIBRARY}")
message(STATUS "Vulkan_LIBRARIES = ${Vulkan_LIBRARIES}")
message(STATUS "Vulkan_INCLUDE_DIRS = ${Vulkan_INCLUDE_DIRS}")

# ----------------------------------------------------------
# Engine
# ----------------------------------------------------------

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
        Engine/*.cpp
)

file(GLOB_RECURSE ENGINE_MODULES CONFIGURE_DEPENDS
        Engine/*.cppm
)

add_library(Engine SHARED)

get_target_property(VULKAN_INCLUDE_DIR Vulkan::Headers INTERFACE_INCLUDE_DIRECTORIES)

target_compile_definitions(Engine
        PRIVATE
        VULKAN_HPP_CXX_MODULE
        VULKAN_HPP_HAS_SPACESHIP_OPERATOR
        VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
        VULKAN_HPP_NO_STRUCT_SETTERS
        VULKAN_HPP_NO_SMART_HANDLE
        VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
)

target_sources(Engine
        PRIVATE
        ${ENGINE_SOURCES}

        PUBLIC
        FILE_SET CXX_MODULES FILES
        ${ENGINE_MODULES}
        ${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm
)

target_include_directories(Engine
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Engine
)

target_link_libraries(Engine
        PUBLIC
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

# ----------------------------------------------------------
# Shaders
# ----------------------------------------------------------

add_subdirectory(Engine/Render/Shaders)

add_dependencies(Game Game_shaders)