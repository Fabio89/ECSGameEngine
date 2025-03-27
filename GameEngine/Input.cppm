export module Input;
import Math;
import Wrapper.Glfw;

export using ::KeyCode;
export using ::KeyAction;
export using ::CursorMode;
export using ::CursorType;

export namespace Input
{
    __declspec(dllexport)
    Vec2 getCursorPosition(GLFWwindow* window);

    __declspec(dllexport)
    bool isKeyDown(KeyCode key);

    __declspec(dllexport)
    bool isKeyJustPressed(KeyCode key);

    __declspec(dllexport)
    bool isKeyJustReleased(KeyCode key);

    __declspec(dllexport)
    void setCursorMode(GLFWwindow* window, CursorMode mode);

    __declspec(dllexport)
    void setCursorType(GLFWwindow* window, CursorType type);
    
    void init(GLFWwindow* window);
    void shutdown();
    void postUpdate();
    using KeyEventCallback = void(*)(KeyCode key, KeyAction action);
    void addKeyEventCallback(KeyEventCallback callback);
}
