module;

#include "EngineExport.h"

export module Editor;
import Core;
import Glfw;

export namespace Editor
{
    ENGINE_API void init();

    ENGINE_API void update(GLFWwindow* window, float deltaTime);
}
