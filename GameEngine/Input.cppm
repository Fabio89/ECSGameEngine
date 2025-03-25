export module Input;
import Math;
import Wrapper.Glfw;

export using ::KeyCode;
export using ::KeyAction;
export using ::CursorMode;

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

    void init(GLFWwindow* window);
    using KeyEventCallback = void(*)(KeyCode key, KeyAction action);
    void addKeyEventCallback(KeyEventCallback callback);
}
