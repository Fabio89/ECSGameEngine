cmake_minimum_required(VERSION 3.10)

project(EcsGameEngine LANGUAGES CXX)

include(


)

# Define the required Vulkan SDK version
set(REQUIRED_VULKAN_SDK_VERSION 283) # Set the required version to 283

# Function to download and install Vulkan SDK
function(install_vulkan_sdk)
    message(STATUS "Vulkan SDK not found or version is incorrect. Downloading and installing...")

    # Determine platform
    if (WIN32)
        set(VULKAN_SDK_URL "https://sdk.lunarg.com/sdk/download/latest/windows/vulkan_sdk.exe")
        set(VULKAN_SDK_INSTALL_COMMAND "${CMAKE_BINARY_DIR}/vulkan_sdk.exe")
    elseif (UNIX AND NOT APPLE)
        set(VULKAN_SDK_URL "https://sdk.lunarg.com/sdk/download/latest/linux/vulkan_sdk.tar.gz")
        set(VULKAN_SDK_INSTALL_COMMAND "tar -xzf ${CMAKE_BINARY_DIR}/vulkan_sdk_installer -C ${CMAKE_BINARY_DIR}/vulkan_sdk")
    elseif (APPLE)
        set(VULKAN_SDK_URL "https://sdk.lunarg.com/sdk/download/latest/mac/vulkan_sdk.dmg")
        set(VULKAN_SDK_INSTALL_COMMAND "hdiutil attach ${CMAKE_BINARY_DIR}/vulkan_sdk_installer && cp -R /Volumes/vulkan_sdk/vulkan_sdk ${CMAKE_BINARY_DIR}/vulkan_sdk && hdiutil detach /Volumes/vulkan_sdk")
    else()
        message(FATAL_ERROR "Unsupported platform")
    endif()

    # Download Vulkan SDK
    file(DOWNLOAD ${VULKAN_SDK_URL} ${CMAKE_BINARY_DIR}/vulkan_sdk.exe
        SHOW_PROGRESS
        STATUS download_status
    )

    list(GET download_status 0 status_code)
    if (NOT status_code EQUAL 0)
        message(FATAL_ERROR "Failed to download Vulkan SDK. Error code: ${status_code}")
    else()
        message(STATUS "Successfully downloaded Vulkan SDK installer.")
    endif()

    # Verify that the installer file exists
    if (NOT EXISTS "${CMAKE_BINARY_DIR}/vulkan_sdk.exe")
        message(FATAL_ERROR "Vulkan SDK installer file not found: ${CMAKE_BINARY_DIR}/vulkan_sdk.exe")
    endif()

    # Install Vulkan SDK
    message(STATUS "Installing Vulkan SDK...")
    execute_process(
        COMMAND ${VULKAN_SDK_INSTALL_COMMAND}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        RESULT_VARIABLE install_result
    )

    if (NOT install_result EQUAL 0)
        message(FATAL_ERROR "Failed to install Vulkan SDK. Error code: ${install_result}")
    endif()

    set(ENV{VULKAN_SDK} "${CMAKE_BINARY_DIR}/vulkan_sdk")
    message(STATUS "Vulkan SDK installed at ${CMAKE_BINARY_DIR}/vulkan_sdk")
endfunction()

# Check if Vulkan SDK is installed and has the correct version
if (DEFINED ENV{VULKAN_SDK} AND EXISTS "$ENV{VULKAN_SDK}")
    # Load Vulkan SDK
    set(VULKAN_SDK_PATH "$ENV{VULKAN_SDK}")
    find_package(Vulkan REQUIRED)

    # Get Vulkan header version
    file(READ "${Vulkan_INCLUDE_DIRS}/vulkan/vulkan_core.h" vulkan_core_h_content)
    string(REGEX MATCH "#define VK_HEADER_VERSION ([0-9]+)" _ ${vulkan_core_h_content})
    set(VULKAN_HEADER_VERSION ${CMAKE_MATCH_1})

    if (VULKAN_HEADER_VERSION LESS ${REQUIRED_VULKAN_SDK_VERSION})
        message(WARNING "Vulkan SDK version ${VULKAN_HEADER_VERSION} is less than the required version ${REQUIRED_VULKAN_SDK_VERSION}. Installing the required version.")
        install_vulkan_sdk()
    else()
        message(STATUS "Vulkan SDK version ${VULKAN_HEADER_VERSION} is sufficient.")
    endif()
else()
    install_vulkan_sdk()
endif()

# Update the Vulkan SDK path if newly installed
set(VULKAN_SDK_PATH "$ENV{VULKAN_SDK}")

# Find Vulkan
find_package(Vulkan REQUIRED)

# Add your project files here
add_executable(EcsGameEngine main.cpp)

# Link Vulkan library
target_include_directories(EcsGameEngine PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(EcsGameEngine Vulkan::Vulkan)
