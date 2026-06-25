module Engine;
import Editor; // TODO: remove this dependency
import EngineSystems;
import Editor.Camera;
import Platform;
import Serialization.Json;

namespace Engine
{
    void runRenderThread();
    void updateEditorCamera(float deltaTime);
    Entity ensureCamera();

    bool shutdownRequested = false;
    std::atomic<bool> engineShuttingDown = false;
    std::thread renderThread;
    Player player;
    WindowHandle window;
}

void Engine::runRenderThread()
{
    renderManager.init(window);

    while (!engineShuttingDown.load())
    {
        renderManager.update();
    }

    renderManager.shutdown();
}

Entity Engine::ensureCamera()
{
    auto hasCamera = [](Entity entity) { return world.hasComponent<CameraComponent>(entity); };
    auto entities = world.getEntitiesRange();
    if (auto cameraEntityIt = std::ranges::find_if(entities, hasCamera); cameraEntityIt != entities.end())
    {
        return *cameraEntityIt;
    }

    const Entity camera = world.createEntity();
    world.addComponent<CameraComponent>(camera, CameraComponent{.fov = 60.f});
    world.addComponent<NameComponent>(camera, "Main Camera");
    world.addComponent<TransformComponent>(camera);

    auto& transform = world.editComponent<TransformComponent>(camera);
    transform.position = {2.f, 2.f, 2.f};
    const Vec3 dir = Math::normalize(-transform.position);
    const Quat rot = Math::rotation(forwardVector(), dir);
    transform.rotation = rot;

    return camera;
}

void Engine::init(const WindowCreateInfo& info)
{
    Platform::init();

    if (!check(!window.isValid(), "Can't create more than one window!"))
        return;

    window = Platform::Window::createWindow(info);

    renderThread = std::thread
    {
        runRenderThread
    };

    Input::init(window);
    EngineComponents::init();
    EngineSystems::init(world);
    Editor::init(world);
}

bool Engine::update(float deltaTime)
{
    if (shutdownRequested || Platform::Window::isWindowClosing(window))
    {
        shutdown();
        return false;
    }

    EngineSystems::update(world, player, deltaTime);

    Input::postUpdate();
    Platform::update();
    Editor::update(world, window, deltaTime);

    return true;
}

void Engine::shutdown()
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

    Platform::Window::destroyWindow(window);
    Platform::shutdown();
    std::cout << "[Application] Shutdown complete!\n";
}

void Engine::setViewport(IVec2 position, IVec2 size)
{
    if (check(window.isValid(), "Can't set viewport offset for null window!"))
    {
        Platform::Window::setWindowPosition(window, position);
        Platform::Window::setWindowSize(window, size);
    }
}

Entity Engine::getEntityUnderCursor()
{
    return Physics::lineTrace(world, Physics::rayFromScreenPosition(world, player, Input::getCursorPosition(window)), TraceChannelFlags::Default);
}

void Engine::openProject(std::filesystem::path path)
{
    EngineSystems::reset();

    Project::open(std::move(path), world);

    Editor::createGizmos(world);

    player.setMainCamera(world, ensureCamera());
}

void Engine::saveCurrentProject()
{
    Project::saveToCurrent(world);
}

Entity Engine::createEntity()
{
    return world.createEntity();
}

void Engine::removeEntity(Entity entity)
{
    return world.removeEntity(entity);
}

bool Engine::isValid(Entity entity)
{
    return world.isValid(entity);
}

bool Engine::hasComponent(Entity entity, ComponentTypeId componentTypeId)
{
    return world.hasComponent(entity, componentTypeId);
}

const ComponentBase& Engine::readComponent(Entity entity, ComponentTypeId componentType)
{
    return world.readComponent(entity, componentType);
}

ComponentBase& Engine::editComponent(Entity entity, ComponentTypeId componentType)
{
    return world.editComponent(entity, componentType);
}

World& Engine::getWorld()
{
    return world;
}

void Engine::printArchetypeStatus()
{
    world.printArchetypeStatus();
}

Player& Engine::getPlayer()
{
    return player;
}

void Engine::updateEditorCamera(float deltaTime)
{
    EditorCamera::update(window, world, player, deltaTime);
}

