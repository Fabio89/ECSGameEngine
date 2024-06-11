Package: vulkan-headers:x64-windows@1.3.280.0

**Host Environment**

- Host: x64-windows
- Compiler: MSVC 19.40.33811.0
-    vcpkg-tool version: 2024-04-23-d6945642ee5c3076addd1a42c331bbf4cfc97457
    vcpkg-scripts version: 14b91796a 2024-06-07 (3 days ago)

**To Reproduce**

`vcpkg install `

**Failure logs**

```
-- Using cached KhronosGroup-Vulkan-Headers-vulkan-sdk-1.3.280.0.tar.gz.
-- Cleaning sources at D:/Dev/vcpkg/buildtrees/vulkan-headers/src/-1.3.280.0-5cd4115c30.clean. Use --editable to skip cleaning for the packages you specify.
-- Extracting source D:/Dev/vcpkg/downloads/KhronosGroup-Vulkan-Headers-vulkan-sdk-1.3.280.0.tar.gz
-- Using source at D:/Dev/vcpkg/buildtrees/vulkan-headers/src/-1.3.280.0-5cd4115c30.clean
CMake Error at C:/Users/fmarchese/AppData/Local/vcpkg/registries/git-trees/47ec3d0d82ea8e9428d59b4c3428ca43cb5ea4c8/portfile.cmake:11 (vcpkg_cmake_configure):
  Unknown CMake command "vcpkg_cmake_configure".
Call Stack (most recent call first):
  scripts/ports.cmake:175 (include)



```

**Additional context**

<details><summary>vcpkg.json</summary>

```
{
  "dependencies": [
    "glfw3",
    "glm",
    {
      "name": "imgui",
      "features": [
        "glfw-binding",
        "vulkan-binding"
      ]
    }
  ]
}

```
</details>
