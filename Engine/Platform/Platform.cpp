module Platform;
import Glfw;
import Window;
import std;

namespace Platform
{
    void init()
    {
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
