export module Engine:Application;
import :Config;
import :Render.RenderThread;
import :Component.Model;
import :Component.Transform;
import :DebugWidget.EntityExplorer;
import :World;
import Wrapper.Windows;

//------------------------------------------------------------------------------------------------------------------------

extern "C" __declspec(dllexport)
inline int getCoolestNumber() { return 22; }

export extern "C" __declspec(dllexport)
void runTest(GLFWwindow* window = nullptr);

export extern "C" __declspec(dllexport)
GLFWwindow* createViewportWindow(HWND parentHwnd, int width, int height);

export extern "C" __declspec(dllexport)
void engineInit(GLFWwindow* window);

export extern "C" __declspec(dllexport)
bool engineUpdate(GLFWwindow* window, float deltaTime);

export extern "C" __declspec(dllexport)
void engineShutdown(GLFWwindow* window);

//------------------------------------------------------------------------------------------------------------------------

void framebufferResizeCallback(GLFWwindow* window, [[maybe_unused]] int width, [[maybe_unused]] int height)
{
    auto renderManager = static_cast<RenderManager*>(glfwGetWindowUserPointer(window));
    renderManager->updateFramebufferSize();
}

GLFWwindow* createWindow(HWND parent, int width, int height);

template<typename T, typename... T_Systems>
void addSystems(World& world)
{
    world.addSystem<T>();

    if constexpr (sizeof...(T_Systems) > 0)
    {
        addSystems<T_Systems...>(world);
    }
}

std::atomic<bool> shouldExit = false;
RenderManager renderManager;
std::thread renderThread;
const ApplicationSettings& settings = Config::getApplicationSettings();
World world{settings, &renderManager};
std::mutex mutex;

void runTestInRenderThread(GLFWwindow* window)
{
    renderManager.init(window);

    auto deltaTime = std::chrono::milliseconds{8};
    while (!shouldExit.load())
    {
        renderManager.update(static_cast<float>(deltaTime.count()));
    }

    renderManager.shutdown();
}

void engineInit(GLFWwindow* window)
{
    renderThread = std::thread
    {
        runTestInRenderThread,
        window
    };
    
    addSystems<ModelSystem, TransformSystem>(world);
    world.createObjectsFromConfig();
    world.addDebugWidget<DebugWidgets::EntityExplorer>();
    world.addDebugWidget<DebugWidgets::ImGuiDemo>();
}

bool engineUpdate(GLFWwindow* window, float deltaTime)
{
    if (glfwWindowShouldClose(window))
    {
        engineShutdown(window);
        return false;
    }
    
    glfwPollEvents();
    world.updateSystems(deltaTime);
    return true;
}

void engineShutdown(GLFWwindow* window)
{
    if (shouldExit.exchange(true))
        return;
    
    std::cout << "[Application] Shutting down...\n";
    
    if (renderThread.joinable())
    {
        std::cout << "[Application] Waiting for render thread to complete...\n"; 
        renderThread.join();
        std::cout << "[Application] Render thread joined!\n"; 
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    std::cout << "[Application] Shutdown complete!\n"; 
}

GLFWwindow* createViewportWindow(HWND parentHwnd, int width, int height)
{
    return createWindow(parentHwnd, width, height);
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
        Wrapper_Windows::SetParent(hwnd, parent);
        Wrapper_Windows::SetWindowLongA(hwnd, GWL_Style, Wrapper_Windows::GetWindowLongA(hwnd, GWL_Style) | WS_Child);
    }

    return window;
}