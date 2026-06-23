module;

#include "EngineExport.h"

export module EditorBridge;
import Core;
import Wrapper.Glfw;

export extern "C"
{
    ENGINE_API void editorInit();

    ENGINE_API void editorUpdate(GLFWwindow* window, float deltaTime);
}
