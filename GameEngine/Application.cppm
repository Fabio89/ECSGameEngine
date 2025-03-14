export module Engine:Application;
import :Config;
import :Render.RenderThread;
import :Component.Model;
import :Component.Transform;
import :DebugWidget.EntityExplorer;
import :World;
import <windows.h>;

export _declspec(dllexport) void runTest();

template<typename T, typename... T_Systems>
void addSystems(World& world)
{
    world.addSystem<T>();

    if constexpr (sizeof...(T_Systems) > 0)
    {
        addSystems<T_Systems...>(world);
    }
}

void framebufferResizeCallback(GLFWwindow* window, [[maybe_unused]] int width, [[maybe_unused]] int height)
{
    auto renderManager = static_cast<RenderManager*>(glfwGetWindowUserPointer(window));
    renderManager->updateFramebufferSize();
}

GLFWwindow* createWindow(HWND parent, int width, int height)
{
    glfwInit();
    glfwWindowHint(glfw::ClientApi, glfw::NoApi);
    glfwWindowHint(glfw::Resizable, glfw::True);
    if (parent)
    {
        glfwWindowHint(glfw::Decorated, glfw::False);
    }
    
    GLFWwindow* window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);

    if (parent)
    {
        const auto hwnd = glfwGetWin32Window(window);
        SetParent(hwnd, parent);
        SetWindowLongA(hwnd, GWL_Style, GetWindowLongA(hwnd, GWL_Style) | WS_Child);
    }

    return window;
}

void runTest()
{
    RenderManager renderManager;

    const ApplicationSettings& settings = Config::getApplicationSettings();
    const LoopSettings loopSettings{.targetFps = settings.targetFps};

    GLFWwindow* window = createWindow(nullptr, settings.resolution.x, settings.resolution.y);
    glfwSetWindowUserPointer(window, &renderManager);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    
    RenderThread renderThread{{&renderManager, window, loopSettings}};
    World world{settings, &renderManager};

    addSystems<ModelSystem, TransformSystem>(world);
    world.addDebugWidget<DebugWidgets::EntityExplorer>();
    world.addDebugWidget<DebugWidgets::ImGuiDemo>();

    world.createObjectsFromConfig();

    performLoop
    (
        loopSettings,
        [&world](float deltaTime)
        {
            glfwPollEvents();
            world.updateSystems(deltaTime);
        },
        [&window]
        {
            return !glfwWindowShouldClose(window);
        }
    );

    renderThread.Close();
    
    glfwDestroyWindow(window);
    glfwTerminate();
}