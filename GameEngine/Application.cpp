module Application;
import DebugWidget.EntityExplorer;
import EngineComponents;
import Math;
import Player;
import Project;
import Render.RenderManager;
import Serialization;
import System;
import World;
import std;

bool shutdownRequested = false;
std::atomic<bool> engineShuttingDown = false;
RenderManager renderManager;
std::thread renderThread;
World world{{&renderManager}};
Player player;
std::vector<KeyEventCallback> keyEventCallbacks;
std::vector<std::unique_ptr<System>> systems;

void framebufferResizeCallback(GLFWwindow* window, [[maybe_unused]] int width, [[maybe_unused]] int height)
{
    auto rm = static_cast<RenderManager*>(glfwGetWindowUserPointer(window));
    rm->updateFramebufferSize();
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    for (KeyEventCallback callback : keyEventCallbacks)
    {
        callback(key, action);
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    keyCallback(window, button, 0, action, mods);
}

GLFWwindow* createWindow(HWND parent, int width, int height);

void addSystem(std::unique_ptr<System> system)
{
    System* systemPtr = systems.emplace_back(std::move(system)).get();
    world.observeOnComponentAdded([systemPtr](Entity entity, ComponentTypeId componentId)
    {
        systemPtr->onComponentAdded(world, entity, componentId);
    });
}

template <typename T>
void addSystem()
{
    addSystem(std::make_unique<T>());
}

template <typename T, typename... T_Systems>
void addSystems(World& world)
{
    addSystem<T>();

    if constexpr (sizeof...(T_Systems) > 0)
    {
        addSystems<T_Systems...>(world);
    }
}

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
    auto hasCamera = [](Entity entity) -> bool { return std::ranges::contains(world.getComponentTypesInEntity(entity), CameraComponent::typeId); };
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

    EngineComponents::init();

    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    
    addSystems<ModelSystem, TransformSystem>(world);
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

    glfwPollEvents();

    if (player.getMainCamera() != invalidId())
    {
        const auto& cameraTransform = world.readComponent<TransformComponent>(player.getMainCamera());
        const auto& cameraSettings = world.readComponent<CameraComponent>(player.getMainCamera());
        renderManager.setCameraTransform(cameraTransform.position, cameraTransform.rotation);
        renderManager.setCameraFov(cameraSettings.fov);
    }

    for (auto& system : systems)
    {
        system->update(deltaTime);
    }
    
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

void addKeyEventCallback(KeyEventCallback callback)
{
    keyEventCallbacks.push_back(callback);
}

void openProject(const char* path)
{
    for (auto& system : systems)
        system->clear();
    
    Project::open(path, world);

    {
        player.setMainCamera(world, ensureCamera());
        const auto& cameraTransform = world.readComponent<TransformComponent>(player.getMainCamera());
        const auto& cameraSettings = world.readComponent<CameraComponent>(player.getMainCamera());
        renderManager.setCameraTransform(cameraTransform.position, cameraTransform.rotation);
        renderManager.setCameraFov(cameraSettings.fov);
    }
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
