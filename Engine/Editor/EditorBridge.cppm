module;

#include "EngineExport.h"

export module EditorBridge;
import Core;
import Wrapper.Glfw;

export extern "C"
{
    ENGINE_API void editTranslation(Entity entity);

    ENGINE_API void editRotation(Entity entity);

    ENGINE_API void editScale(Entity entity);

    ENGINE_API void stopEditing();

    ENGINE_API void editorInit();

    ENGINE_API void editorUpdate(GLFWwindow* window, float deltaTime);
}
