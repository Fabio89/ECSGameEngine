#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_DIR"

echo "Installing Fedora dependencies..."

sudo dnf install -y \
    cmake \
    gcc-c++ \
    glslang \
    glslc \
    libXcursor-devel \
    libXi-devel \
    libXinerama-devel \
    libXrandr-devel \
    libasan \
    libxkbcommon-devel \
    mesa-libGL-devel \
    ninja-build \
    pkgconf-pkg-config \
    vulkan-headers \
    vulkan-loader \
    vulkan-loader-devel \
    vulkan-tools \
    vulkan-validation-layers \
    vulkan-validation-layers-devel \
    wayland-devel

sudo dnf install -y glslc || true

echo "Setting up vcpkg..."

VCPKG_DIR="$PROJECT_DIR/vcpkg"

if [ ! -d "$VCPKG_DIR/.git" ]; then
    echo "Cloning vcpkg..."
    rm -rf "$VCPKG_DIR"
    git clone https://github.com/microsoft/vcpkg.git "$VCPKG_DIR"
fi

if [ ! -f "$VCPKG_DIR/vcpkg" ]; then
    echo "Bootstrapping vcpkg..."
    "$VCPKG_DIR/bootstrap-vcpkg.sh"
fi

echo "Configuring project..."

if [ ! -f "$PROJECT_DIR/CMakePresets.json" ]; then
    echo "Could not find CMakePresets.json"
    exit 1
fi

cmake --preset debug

echo "Done."