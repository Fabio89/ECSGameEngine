module;
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

export module Wrapper.Glfw;
import vulkan_hpp;
import std;

export using ::GLFWwindow;
export using ::glfwGetFramebufferSize;
export using ::glfwInit;
export using ::glfwWindowHint;
export using ::glfwCreateWindow;
export using ::glfwSetWindowPos;
export using ::glfwGetWindowSize;
export using ::glfwSetWindowSize;
export using ::glfwSetWindowUserPointer;
export using ::glfwSetFramebufferSizeCallback;
export using ::glfwPollEvents;
export using ::glfwDestroyWindow;
export using ::glfwTerminate;
export using ::glfwWindowShouldClose;
export using ::glfwWaitEvents;
export using ::glfwGetRequiredInstanceExtensions;
export using ::glfwGetWindowUserPointer;
export using ::glfwCreateWindowSurface;
export using ::glfwGetWin32Window;
export using ::glfwMakeContextCurrent;
export using ::glfwSwapInterval;
export using ::glfwSetKeyCallback;
export using ::glfwSetMouseButtonCallback;
export using ::glfwSetCursorPosCallback;
export using ::glfwSetScrollCallback;
export using ::glfwGetCursorPos;

export enum GetWindowLongOption
{
    GWL_Style = GWL_STYLE
};

export enum WindowStyle
{
    WS_Child = WS_CHILD
};

export namespace glfw
{
    constexpr auto ClientApi = GLFW_CLIENT_API;
    constexpr auto NoApi = GLFW_NO_API;
    constexpr auto Decorated = GLFW_DECORATED;
    constexpr auto Resizable = GLFW_RESIZABLE;
    constexpr auto True = GLFW_TRUE;
    constexpr auto False = GLFW_FALSE;

    vk::Result createWindowSurface(vk::Instance instance, GLFWwindow* window, const vk::AllocationCallbacks* allocator, vk::SurfaceKHR* surface)
    {
        return static_cast<vk::Result>(glfwCreateWindowSurface(instance, window, reinterpret_cast<const VkAllocationCallbacks*>(allocator),
                                                               reinterpret_cast<VkSurfaceKHR*>(surface)));
    }
}

export enum class KeyCode
{
    MouseButton1 = 0,
    MouseButton2 = 1,
    MouseButton3 = 2,
    MouseButton4 = 3,
    MouseButton5 = 4,
    MouseButton6 = 5,
    MouseButton7 = 6,
    MouseButton8 = 7,
    MouseButtonLeft = MouseButton1,
    MouseButtonRight = MouseButton2,
    MouseButtonMiddle = MouseButton3,

    Space = 32,
    Apostrophe = 39,
    Comma = 44,
    Minus = 45,
    Period = 46,
    Slash = 47,
    Num0 = 48,
    Num1 = 49,
    Num2 = 50,
    Num3 = 51,
    Num4 = 52,
    Num5 = 53,
    Num6 = 54,
    Num7 = 55,
    Num8 = 56,
    Num9 = 57,
    Semicolon = 59,
    Equal = 61,
    A = 65,
    B = 66,
    C = 67,
    D = 68,
    E = 69,
    F = 70,
    G = 71,
    H = 72,
    I = 73,
    J = 74,
    K = 75,
    L = 76,
    M = 77,
    N = 78,
    O = 79,
    P = 80,
    Q = 81,
    R = 82,
    S = 83,
    T = 84,
    U = 85,
    V = 86,
    W = 87,
    X = 88,
    Y = 89,
    Z = 90,
    LeftBracket = 91,
    Backslash = 92,
    RightBracket = 93,
    GraveAccent = 96,
    World1 = 161,
    World2 = 162,
    Escape = 256,
    Enter = 257,
    Tab = 258,
    Backspace = 259,
    Insert = 260,
    Delete = 261,
    Right = 262,
    Left = 263,
    Down = 264,
    Up = 265,
    PageUp = 266,
    PageDown = 267,
    Home = 268,
    End = 269,
    CapsLock = 280,
    ScrollLock = 281,
    NumLock = 282,
    PrintScreen = 283,
    Pause = 284,
    F1 = 290,
    F2 = 291,
    F3 = 292,
    F4 = 293,
    F5 = 294,
    F6 = 295,
    F7 = 296,
    F8 = 297,
    F9 = 298,
    F10 = 299,
    F11 = 300,
    F12 = 301,
    F13 = 302,
    F14 = 303,
    F15 = 304,
    F16 = 305,
    F17 = 306,
    F18 = 307,
    F19 = 308,
    F20 = 309,
    F21 = 310,
    F22 = 311,
    F23 = 312,
    F24 = 313,
    F25 = 314,
    Kp0 = 320,
    Kp1 = 321,
    Kp2 = 322,
    Kp3 = 323,
    Kp4 = 324,
    Kp5 = 325,
    Kp6 = 326,
    Kp7 = 327,
    Kp8 = 328,
    Kp9 = 329,
    KpDecimal = 330,
    KpDivide = 331,
    KpMultiply = 332,
    KpSubtract = 333,
    KpAdd = 334,
    KpEnter = 335,
    KpEqual = 336,
    LeftShift = 340,
    LeftControl = 341,
    LeftAlt = 342,
    LeftSuper = 343,
    RightShift = 344,
    RightControl = 345,
    RightAlt = 346,
    RightSuper = 347,
    Menu = 348,
};
export constexpr int KeyCodeCount = static_cast<int>(KeyCode::Menu) + 1;

export enum class KeyAction
{
    Release = GLFW_RELEASE,
    Press = GLFW_PRESS,
    Repeat = GLFW_REPEAT,
};

export enum class KeyMod
{
    Shift = 0x0001,
    Control = 0x0002,
    Alt = 0x0004,
    Super = 0x0008,
    CapsLock = 0x0010,
    NumLock = 0x0020,
};
