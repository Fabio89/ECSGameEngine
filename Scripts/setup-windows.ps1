$ErrorActionPreference = "Stop"

Write-Host "Setting up vcpkg..."

$VCPKG_DIR="$env:USERPROFILE\vcpkg"

if (!(Test-Path $VCPKG_DIR)) {
    git clone https://github.com/microsoft/vcpkg.git $VCPKG_DIR
}

if (!(Test-Path "$VCPKG_DIR\vcpkg.exe")) {
    & "$VCPKG_DIR\bootstrap-vcpkg.bat"
}

Write-Host "Configuring project..."

cmake --preset windows-debug

Write-Host "Done."