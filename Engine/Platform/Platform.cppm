export module Platform;
import Glfw;

export namespace Platform
{
    void init()
    {
        glfwInit();
    }

    void update()
    {
        glfwPollEvents();
    }

    void shutdown()
    {
        glfwTerminate();
    }
}
