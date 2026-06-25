export module Window;
import Core;
import Glfw;
import Math;
import std;

export struct WindowHandle : Id<WindowHandle> {};

export enum class WindowMode
{
    Standalone,
    Embedded
};

export struct WindowCreateInfo
{
    int width = 1280;
    int height = 720;
    WindowMode mode = WindowMode::Standalone;
};

export using ::KeyCodeCount;
export using ::KeyCode;
export using ::KeyAction;
export using ::CursorMode;
export using ::CursorType;

export namespace Platform::Window
{
    WindowHandle createWindow(WindowCreateInfo info);
    void destroyWindow(WindowHandle window);

    Vec2 getCursorPosition(WindowHandle window);
    void setCursorMode(WindowHandle window, CursorMode mode);
    void setCursorType(WindowHandle window, CursorType type);
    void destroyCursors();

    IVec2 getWindowSize(WindowHandle window);
    void setWindowSize(WindowHandle window, IVec2 size);

    IVec2 getWindowPosition(WindowHandle window);
    void setWindowPosition(WindowHandle window, IVec2 position);

    bool isWindowClosing(WindowHandle window);

    using KeyFunction = void(*)(KeyCode key, KeyAction action);
    void setKeyCallback(WindowHandle window, KeyFunction callback);

    using MouseButtonFunction = void(*)(KeyCode button, KeyAction action);
    void setMouseButtonCallback(WindowHandle window, MouseButtonFunction callback);

    GLFWwindow* getGlfwWindow(WindowHandle window);
}
