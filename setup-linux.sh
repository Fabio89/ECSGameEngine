#!/usr/bin/env bash
set -e

echo "Installing Fedora dependencies..."

sudo dnf install -y \
    ninja-build \
    cmake \
    gcc-c++ \
    pkgconf-pkg-config \
    vulkan-headers \
    vulkan-loader-devel \
    vulkan-validation-layers \
    vulkan-validation-layers-devel \
    vulkan-tools \
    glslang \
    glslc \
    libasan \
    libXinerama-devel \
    libXcursor-devel \
    libXrandr-devel \
    libXi-devel

sudo dnf install -y glslc || true

echo "Setting up vcpkg..."

VCPKG_DIR="$HOME/.local/vcpkg"

if [ ! -d "$VCPKG_DIR" ]; then
    git clone https://github.com/microsoft/vcpkg.git "$VCPKG_DIR"
fi

if [ ! -f "$VCPKG_DIR/vcpkg" ]; then
    "$VCPKG_DIR/bootstrap-vcpkg.sh"
fi

echo "Configuring project..."
cmake --preset linux-debug

echo "Done."