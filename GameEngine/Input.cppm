module;

#include "EngineExport.h"

export module Input;
import Math;
import Wrapper.Glfw;

export using ::GlfwKeyCode;
export using ::KeyAction;
export using ::CursorMode;
export using ::CursorType;

export namespace Input
{
    ENGINE_API
    Vec2 getCursorPosition(GLFWwindow* window);

    ENGINE_API
    bool isKeyDown(GlfwKeyCode key);

    ENGINE_API
    bool isKeyJustPressed(GlfwKeyCode key);

    ENGINE_API
    bool isKeyJustReleased(GlfwKeyCode key);

    ENGINE_API
    void setCursorMode(GLFWwindow* window, CursorMode mode);

    ENGINE_API
    void setCursorType(GLFWwindow* window, CursorType type);
    
    void init(GLFWwindow* window);
    void shutdown();
    void postUpdate();
    using KeyEventCallback = void(*)(GlfwKeyCode key, KeyAction action);
    void addKeyEventCallback(KeyEventCallback callback);
}
