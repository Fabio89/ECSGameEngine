module Input;
import std;

namespace Input
{
    std::vector<KeyEventCallback> keyEventCallbacks;

    std::bitset<KeyCodeCount> heldKeys{};
    std::bitset<KeyCodeCount> justPressedKeys{};
    std::bitset<KeyCodeCount> justReleasedKeys{};

    void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
}

void Input::init(GLFWwindow* window)
{
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
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

void Input::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
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
}

void Input::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    keyCallback(window, button, 0, action, mods);
}
