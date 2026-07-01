module Input;
import Core;
import Window;

namespace
{
    std::bitset<KeyCodeCount> heldKeys{};
    std::bitset<KeyCodeCount> justPressedKeys{};
    std::bitset<KeyCodeCount> justReleasedKeys{};

    Vec2 cursorScreenPositionLastFrame;
    bool firstFrame = true;
}

namespace Input
{
    void keyCallback(KeyCode key, KeyAction action);
    void mouseButtonCallback(KeyCode button, KeyAction action);
}

Vec2 Input::getCursorScreenPosition(WindowHandle window)
{
    return Platform::Window::getCursorPosition(window);
}

Vec2 Input::getCursorDelta(WindowHandle window)
{
    return firstFrame ? Vec2{} : getCursorScreenPosition(window) - cursorScreenPositionLastFrame;
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

void Input::setCursorMode(WindowHandle window, CursorMode mode)
{
    return Platform::Window::setCursorMode(window, mode);
}

void Input::setCursorType(WindowHandle window, CursorType type)
{
    return Platform::Window::setCursorType(window, type);
}

void Input::init(WindowHandle window)
{
    Platform::Window::setKeyCallback(window, keyCallback);
    Platform::Window::setMouseButtonCallback(window, mouseButtonCallback);
}

void Input::postUpdate(WindowHandle window)
{
    justPressedKeys.reset();
    justReleasedKeys.reset();

    cursorScreenPositionLastFrame = getCursorScreenPosition(window);
    firstFrame = false;
}

void Input::keyCallback(KeyCode key, KeyAction action)
{
    const auto keyValue = static_cast<std::size_t>(key);
    heldKeys[keyValue] = action == KeyAction::Press || action == KeyAction::Repeat;
    justPressedKeys[keyValue] = action == KeyAction::Press;
    justReleasedKeys[keyValue] = action == KeyAction::Release;

    std::string keyActionStr = action == KeyAction::Press ? "Press" : action == KeyAction::Release ? "Release" : "Repeat";

    log(std::format("Key: {}, Action: {}", keyValue, keyActionStr));
}

void Input::mouseButtonCallback(KeyCode button, KeyAction action)
{
    keyCallback(button, action);
}
