module Engine:Application;
import :Application;
import :Components;
import :DebugWidget.EntityExplorer;
import :Project;
import :Render.RenderManager;
import :World;
import Wrapper.RapidJson;

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
World world{{&renderManager}};

void runRenderThread(GLFWwindow* window)
{
    renderManager.init(window);

    while (!shouldExit.load())
    {
        renderManager.update();
    }

    renderManager.shutdown();
}

void engineInit(GLFWwindow* window)
{
    renderThread = std::thread
    {
        runRenderThread,
        window
    };
    
    EngineComponents::init();

    addSystems<ModelSystem, TransformSystem>(world);
    // world.addDebugWidget<DebugWidgets::EntityExplorer>();
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

void setViewport(GLFWwindow* window, int x, int y, int width, int height)
{
    if (check(window, "Can't set viewport offset for null window!"))
    {
        glfwSetWindowPos(window, x, y);
        glfwSetWindowSize(window, width, height);
    }
}

void openProject(const char* path)
{
    Project::open(path, world);
}

void serializeScene(char* buffer, int bufferSize)
{
    JsonDocument doc;
    doc.Swap(world.serializeScene(doc.GetAllocator()).Move());

    Json::StringBuffer jsonBuffer{};
    Json::PrettyWriter writer{jsonBuffer};
    writer.SetMaxDecimalPlaces(Json::defaultFloatPrecision);
    doc.Accept(writer);

    log(std::format("Serialized scene:\n\n{}", jsonBuffer.GetString()));
    try
    {
        std::memcpy(buffer, jsonBuffer.GetString(), bufferSize);
    }
    catch (std::exception& e)
    {
        report(std::format("Crash in serializeScene std::memcpy! {}", e.what()));
    }
}

void patchEntity(Entity entity, const char* json)
{
    //world
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
    glfwSwapInterval(1);
    
    if (parent)
    {
        const auto hwnd = glfwGetWin32Window(window);
        Wrapper_Windows::SetParent(hwnd, parent);
        Wrapper_Windows::SetWindowLongA(hwnd, GWL_Style, Wrapper_Windows::GetWindowLongA(hwnd, GWL_Style) | WS_Child);
    }

    return window;
}
