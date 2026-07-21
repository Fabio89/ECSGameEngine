export module Window;
export import Core;
import Geometry;
import Glfw;
import Math;

export struct WindowHandle : Id<struct WindowHandleTag> {};

template<>
struct std::hash<WindowHandle>
{
    constexpr std::size_t operator()(const WindowHandle& id) const noexcept { return std::hash<Id<WindowHandleTag>>{}(id); }
};

export enum class WindowMode
{
    Windowed,
    Maximized,
    Fullscreen,
    Embedded
};

export struct WindowCreateInfo
{
    Size2D size;
    WindowMode mode = WindowMode::Maximized;
};

export using ::KeyCodeCount;
export using ::KeyCode;
export using ::KeyAction;
export using ::CursorMode;
export using ::CursorType;

export namespace Platform::Window
{
    void init();
    void update();
    void shutdown();

    WindowHandle createWindow(WindowCreateInfo info);
    void destroyWindow(WindowHandle window);

    Vec2 getCursorPosition(WindowHandle window);
    Vec2 getMouseScrollDelta(WindowHandle window);
    CursorMode getCursorMode(WindowHandle window);
    void setCursorMode(WindowHandle window, CursorMode mode);
    void setCursorType(WindowHandle window, CursorType type);

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
