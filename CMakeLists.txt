cmake_minimum_required(VERSION 4.3)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")

project(
        EcsGameEngine
        VERSION 1.0
        LANGUAGES CXX
)

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

if(ENABLE_ASAN)
    if(MSVC)
        add_compile_options(/fsanitize=address)
        add_link_options(/fsanitize=address)
    else()
        add_compile_options(
                -fsanitize=address
                -fno-omit-frame-pointer
        )

        add_link_options(
                -fsanitize=address
        )
    endif()
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS_DEVELOPMENT "/Od /Zi")
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
else()
    set(CMAKE_CXX_FLAGS_DEVELOPMENT "-Og -g")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

set(CMAKE_CONFIGURATION_TYPES "Debug;Development;Release" CACHE STRING "" FORCE)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(UNIX AND NOT APPLE)
    set(Vulkan_LIBRARY "/usr/lib64/libvulkan.so.1")
endif()

find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

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

set_target_properties(Engine PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

get_target_property(VULKAN_INCLUDE_DIR Vulkan::Headers INTERFACE_INCLUDE_DIRECTORIES)

target_compile_definitions(Engine
        PRIVATE
        ENGINE_BUILD
        VULKAN_HPP_CXX_MODULE
        VULKAN_HPP_HAS_SPACESHIP_OPERATOR
        VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
        VULKAN_HPP_NO_STRUCT_SETTERS
        VULKAN_HPP_NO_SMART_HANDLE
        VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
)

target_compile_features(Engine PUBLIC cxx_std_23)

target_compile_options(Engine PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

target_sources(Engine
        PRIVATE
        ${ENGINE_SOURCES}

        PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
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

set(GAME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_target_properties(Game PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${GAME_OUTPUT_DIRECTORY}"
)

target_link_libraries(Game
        PRIVATE
        Engine
)

target_compile_features(Game PUBLIC cxx_std_23)

target_compile_options(Game PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

set_property(TARGET Game PROPERTY CXX_MODULE_STD ON)


# ----------------------------------------------------------
# Shaders
# ----------------------------------------------------------

add_subdirectory(Engine/Rendering/Shaders)

add_dependencies(Game Game_shaders)