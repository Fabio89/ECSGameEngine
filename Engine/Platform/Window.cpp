module Window;
import Glfw;

namespace Platform::Window
{
    struct WindowData
    {
        GLFWwindow* glfwHandle{};
        std::vector<KeyFunction> keyEventCallbacks;
        std::vector<MouseButtonFunction> mouseEventCallbacks;
    };

    std::unordered_map<GLFWwindow*, WindowHandle> glfwWindowToHandle;
    std::unordered_map<WindowHandle, WindowData> windows;
    WindowHandle::ValueType nextWindowHandle = 0;
    std::unordered_map<CursorType, GLFWcursor*> cursorTypes;

    WindowData& getWindowData(WindowHandle handle);
    void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
}

void Platform::Window::init()
{
}

void Platform::Window::shutdown()
{
    for (GLFWcursor* cursorType : cursorTypes | std::views::values)
    {
        glfwDestroyCursor(cursorType);
    }
    cursorTypes.clear();
}

WindowHandle Platform::Window::createWindow(WindowCreateInfo info)
{
    glfwWindowHint(glfw::ClientApi, glfw::NoApi);
    glfwWindowHint(glfw::Resizable, glfw::Enabled);

    if (info.mode == WindowMode::Embedded)
    {
        glfwWindowHint(glfw::Decorated, glfw::Disabled);
    }

    GLFWwindow* glfwWindow = glfwCreateWindow
    (
        info.width,
        info.height,
        "Engine",
        nullptr,
        nullptr
    );

    glfwSetKeyCallback(glfwWindow, keyCallback);
    glfwSetMouseButtonCallback(glfwWindow, mouseButtonCallback);

    const WindowHandle handle{nextWindowHandle++};
    windows[handle].glfwHandle = glfwWindow;
    glfwWindowToHandle[glfwWindow] = handle;

    return handle;
}

void Platform::Window::destroyWindow(WindowHandle window)
{
    WindowData& data = getWindowData(window);
    glfwDestroyWindow(data.glfwHandle);
    glfwWindowToHandle.erase(data.glfwHandle);
    data = {};
    windows.erase(window);
}

Vec2 Platform::Window::getCursorPosition(WindowHandle window)
{
    double x, y;
    glfwGetCursorPos(getWindowData(window).glfwHandle, &x, &y);
    return {x, y};
}

void Platform::Window::setCursorMode(WindowHandle window, CursorMode mode)
{
    glfwSetInputMode(getWindowData(window).glfwHandle, static_cast<int>(InputMode::Cursor), static_cast<int>(mode));
}

void Platform::Window::setCursorType(WindowHandle window, CursorType type)
{
    log(std::format("Set cursor type: {}", static_cast<int>(type)));

    GLFWcursor* cursor;
    if (auto it = cursorTypes.find(type); it != cursorTypes.end())
    {
        cursor = it->second;
    }
    else
    {
        cursor = glfwCreateStandardCursor(static_cast<int>(type));
        cursorTypes[type] = cursor;
    }
    glfwSetCursor(getWindowData(window).glfwHandle, cursor);
}

IVec2 Platform::Window::getWindowSize(WindowHandle window)
{
    IVec2 size;
    glfwGetWindowSize(getWindowData(window).glfwHandle, &size.x, &size.y);
    return size;
}

void Platform::Window::setWindowSize(WindowHandle window, IVec2 size)
{
    glfwSetWindowSize(getWindowData(window).glfwHandle, size.x, size.y);
}

IVec2 Platform::Window::getWindowPosition(WindowHandle window)
{
    IVec2 position;
    glfwGetWindowPos(getWindowData(window).glfwHandle, &position.x, &position.y);
    return position;
}

void Platform::Window::setWindowPosition(WindowHandle window, IVec2 position)
{
    glfwSetWindowPos(getWindowData(window).glfwHandle, position.x, position.y);
}

bool Platform::Window::isWindowClosing(WindowHandle window)
{
    return glfwWindowShouldClose(getWindowData(window).glfwHandle);
}

void Platform::Window::setKeyCallback(WindowHandle window, KeyFunction callback)
{
    getWindowData(window).keyEventCallbacks.push_back(std::move(callback));
}

void Platform::Window::setMouseButtonCallback(WindowHandle window, MouseButtonFunction callback)
{
    getWindowData(window).mouseEventCallbacks.push_back(std::move(callback));
}

GLFWwindow* Platform::Window::getGlfwWindow(WindowHandle window)
{
    return getWindowData(window).glfwHandle;
}

Platform::Window::WindowData& Platform::Window::getWindowData(WindowHandle handle)
{
    if (auto it = windows.find(handle); it != windows.end())
        return it->second;

    fatalError(std::format("Couldn't find window with handle: {}", handle.value));
    static WindowData emptyWindow{};
    return emptyWindow;
}

void Platform::Window::keyCallback([[maybe_unused]] GLFWwindow* window, int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods)
{
    if (auto it = glfwWindowToHandle.find(window); it != glfwWindowToHandle.end())
    {
        const KeyCode keyCode = static_cast<KeyCode>(key);
        const KeyAction keyAction = static_cast<KeyAction>(action);

        for (const KeyFunction& callback : windows[it->second].keyEventCallbacks)
        {
            callback(keyCode, keyAction);
        }
    }
}

void Platform::Window::mouseButtonCallback([[maybe_unused]] GLFWwindow* window, int button, int action, [[maybe_unused]] int mods)
{
    if (auto it = glfwWindowToHandle.find(window); it != glfwWindowToHandle.end())
    {
        const KeyCode keyCode = static_cast<KeyCode>(button);
        const KeyAction keyAction = static_cast<KeyAction>(action);

        for (const MouseButtonFunction& callback : windows[it->second].mouseEventCallbacks)
        {
            callback(keyCode, keyAction);
        }
    }
}