module Input;
import Log;
import Wrapper.Glfw;
import std;

namespace Input
{
    std::vector<KeyEventCallback> keyEventCallbacks;

    std::bitset<KeyCodeCount> heldKeys{};
    std::bitset<KeyCodeCount> justPressedKeys{};
    std::bitset<KeyCodeCount> justReleasedKeys{};
    std::unordered_map<CursorType, GLFWcursor*> cursorTypes;

    void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
}

void Input::init(GLFWwindow* window)
{
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
}

void Input::shutdown()
{
    for (GLFWcursor* cursorType : cursorTypes | std::views::values)
    {
        glfwDestroyCursor(cursorType);
    }
    cursorTypes.clear();
}

void Input::postUpdate()
{
    justPressedKeys.reset();
    justReleasedKeys.reset();
}

void Input::addKeyEventCallback(KeyEventCallback callback)
{
    keyEventCallbacks.push_back(callback);
}

Vec2 Input::getCursorPosition(GLFWwindow* window)
{
    double x, y;
    IVec2 size;
    glfwGetCursorPos(window, &x, &y);
    glfwGetWindowSize(window, &size.x, &size.y);
    return {x / size.x, y / size.y};
}

bool Input::isKeyDown(KeyCode key)
{
    return heldKeys[static_cast<int>(key)];
}

bool Input::isKeyJustPressed(KeyCode key)
{
    return justPressedKeys[static_cast<int>(key)];
}

bool Input::isKeyJustReleased(KeyCode key)
{
    return justReleasedKeys[static_cast<int>(key)];
}

void Input::setCursorMode(GLFWwindow* window, CursorMode mode)
{
    glfwSetInputMode(window, static_cast<int>(InputMode::Cursor), static_cast<int>(mode));
}

void Input::setCursorType(GLFWwindow* window, CursorType type)
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
    glfwSetCursor(window, cursor);
}

void Input::keyCallback([[maybe_unused]] GLFWwindow* window, int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods)
{
    const KeyCode keyCode = static_cast<KeyCode>(key);
    const KeyAction keyAction = static_cast<KeyAction>(action);

    for (KeyEventCallback callback : keyEventCallbacks)
    {
        callback(keyCode, keyAction);
    }

    heldKeys[key] = keyAction == KeyAction::Press || keyAction == KeyAction::Repeat;
    justPressedKeys[key] = keyAction == KeyAction::Press;
    justReleasedKeys[key] = keyAction == KeyAction::Release;

    std::string keyActionStr = keyAction == KeyAction::Press ? "Press" : keyAction == KeyAction::Release ? "Release" : "Repeat";

    log(std::format("Key: {}, Action: {}", static_cast<int>(keyCode), keyActionStr));
}

void Input::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    keyCallback(window, button, 0, action, mods);
}
