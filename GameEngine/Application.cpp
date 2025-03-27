module Application;
import DebugWidget.EntityExplorer;
import EngineComponents;
import EngineSystems;
import Feature.DebugFlyCamera;
import Input;
import Math;
import Physics;
import Player;
import Project;
import Render.RenderManager;
import Serialization;
import World;

bool shutdownRequested = false;
std::atomic<bool> engineShuttingDown = false;
RenderManager renderManager;
std::thread renderThread;
World world{{&renderManager}};
Player player;

void framebufferResizeCallback(GLFWwindow* window, [[maybe_unused]] int width, [[maybe_unused]] int height)
{
    auto rm = static_cast<RenderManager*>(glfwGetWindowUserPointer(window));
    rm->updateFramebufferSize();
}

GLFWwindow* createWindow(HWND parent, int width, int height);

void runRenderThread(GLFWwindow* window)
{
    renderManager.init(window);

    while (!engineShuttingDown.load())
    {
        renderManager.update();
    }

    renderManager.shutdown();
}

Entity ensureCamera()
{
    auto hasCamera = [](Entity entity) { return world.hasComponent<CameraComponent>(entity); };
    auto entities = world.getEntitiesRange();
    if (auto cameraEntityIt = std::ranges::find_if(entities, hasCamera); cameraEntityIt != entities.end())
    {
        return *cameraEntityIt;
    }

    const Entity camera = world.createEntity();
    world.addComponent<CameraComponent>(camera, CameraComponent{.fov = 60.f});
    world.addComponent<TransformComponent>(camera);

    auto& transform = world.editComponent<TransformComponent>(camera);
    transform.position = {2.f, 2.f, 2.f};
    const Vec3 dir = Math::normalize(-transform.position);
    const Quat rot = Math::rotation(forwardVector(), dir);
    transform.rotation = rot;

    return camera;
}

void engineInit(GLFWwindow* window)
{
    renderThread = std::thread
    {
        runRenderThread,
        window
    };

    Input::init(window);
    EngineComponents::init();
    EngineSystems::init(world);
    // world.addDebugWidget<DebugWidgets::EntityExplorer>();
    //world.addDebugWidget<DebugWidgets::ImGuiDemo>();
}

bool engineUpdate(GLFWwindow* window, float deltaTime)
{
    if (shutdownRequested || glfwWindowShouldClose(window))
    {
        engineShutdown(window);
        return false;
    }

    EngineSystems::update(world, player, deltaTime);
    
    Input::postUpdate();
    glfwPollEvents();

    return true;
}

void engineShutdown(GLFWwindow* window)
{
    if (engineShuttingDown.exchange(true))
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

void addKeyEventCallback(Input::KeyEventCallback callback)
{
    Input::addKeyEventCallback(reinterpret_cast<Input::KeyEventCallback>(callback));
}

Entity getEntityUnderCursor(GLFWwindow* window)
{
    return Physics::lineTrace(world, Physics::rayFromScreenPosition(world, player, getCursorPosition(window)));
}

Vec2 getCursorPosition(GLFWwindow* window)
{
    return Input::getCursorPosition(window);
}

void openProject(const char* path)
{
    EngineSystems::reset();

    Project::open(path, world);

    player.setMainCamera(world, ensureCamera());
}

void saveCurrentProject()
{
    Project::saveToCurrent(world);
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
    std::memcpy(buffer, jsonBuffer.GetString(), bufferSize);
}

void patchEntity(Entity entity, const char* json)
{
    log(std::format("Patching entity '{}' with:\n{}", entity, json));

    JsonDocument document;
    document.Parse(json);

    if (document.HasParseError())
    {
        report(std::format("JSON parse error while trying to patch entity '{}'! Patch:\n{}", entity, json));
        return;
    }

    world.patchEntity(entity, document);
}

World& getWorld()
{
    return world;
}

Player& getPlayer()
{
    return player;
}

ComponentBase& editComponent(Entity entity, ComponentTypeId typeId)
{
    return const_cast<ComponentBase&>(world.readComponent(entity, typeId));
}

void updateDebugCamera(GLFWwindow* window, float deltaTime)
{
    DebugCamera::update(window, world, player, deltaTime);
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
