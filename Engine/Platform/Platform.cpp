module;
#include "GLFW/glfw3.h"

module Platform;
import Glfw;
import Window;
import std;

namespace Platform
{
    void init()
    {
        glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
        glfwInit();
        Window::init();
    }

    void update()
    {
        Window::update();
        glfwPollEvents();
    }

    void shutdown()
    {
        Window::shutdown();
        glfwTerminate();
    }
}
