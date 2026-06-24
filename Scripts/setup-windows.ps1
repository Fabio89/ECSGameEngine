$ErrorActionPreference = "Stop"

function Ensure-Command
{
    param(
        [string]$Command,
        [string]$WingetId
    )

    if (!(Get-Command $Command -ErrorAction SilentlyContinue))
    {
        Write-Host "Installing $Command..."

        winget install `
        --exact `
        --id $WingetId `
        --accept-package-agreements `
        --accept-source-agreements
    }
}

Write-Host ""
Write-Host "========================================"
Write-Host " ECS Game Engine Setup"
Write-Host "========================================"
Write-Host ""

#
# Core tools
#

Ensure-Command git "Git.Git"
Ensure-Command cmake "Kitware.CMake"
Ensure-Command ninja "Ninja-build.Ninja"

$RequiredCMakeVersion = [Version]"4.3"
$CurrentVersionText = (cmake --version | Select-Object -First 1)

if ($CurrentVersionText -match '(\d+\.\d+\.\d+)')
{
    $CurrentVersion = [Version]$Matches[1]

    if ($CurrentVersion -lt $RequiredCMakeVersion)
    {
        Write-Host "Updating CMake..."

        winget upgrade Kitware.CMake `
            --accept-package-agreements `
            --accept-source-agreements
    }
}

#
# Vulkan SDK
#

if (-not $env:VULKAN_SDK)
{
    Write-Host "Installing Vulkan SDK..."

    winget install `
    --exact `
    --id LunarG.VulkanSDK `
    --accept-package-agreements `
    --accept-source-agreements

    Write-Host ""
    Write-Host "Vulkan SDK installed."
    Write-Host "You may need to restart your terminal or IDE."
    Write-Host ""
}

#
# Project paths
#

$SCRIPT_DIR = Split-Path -Parent $MyInvocation.MyCommand.Path
$PROJECT_DIR = Split-Path $SCRIPT_DIR -Parent

Set-Location $PROJECT_DIR

#
# vcpkg
#

$VCPKG_DIR = Join-Path $PROJECT_DIR "vcpkg"

Write-Host "Setting up vcpkg..."

if (!(Test-Path "$VCPKG_DIR.git"))
{
    Write-Host "Cloning vcpkg..."

    if (Test-Path $VCPKG_DIR)
    {
        Remove-Item $VCPKG_DIR -Recurse -Force
    }

    git clone https://github.com/microsoft/vcpkg.git $VCPKG_DIR
}

if (!(Test-Path "$VCPKG_DIR\vcpkg.exe"))
{
    Write-Host "Bootstrapping vcpkg..."

    & "$VCPKG_DIR\bootstrap-vcpkg.bat"
}

#
# Configure project
#

if (!(Test-Path "$PROJECT_DIR\CMakePresets.json"))
{
    throw "Could not find CMakePresets.json"
}

Write-Host "Configuring project..."

cmake --preset debug

Write-Host ""
Write-Host "========================================"
Write-Host " Setup complete"
Write-Host "========================================"
Write-Host ""
