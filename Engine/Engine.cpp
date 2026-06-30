module Engine;
import EngineSystems;
import Platform;
import Thread;

namespace Engine
{
    void runRenderThread();
    Entity ensureCamera();

    bool shutdownRequested = false;
    std::atomic engineShuttingDown = false;
    std::thread renderThread;
    Player player;
    WindowHandle window;
    EventBus eventBus;
    ThreadOwned threadChecker;
}

void Engine::runRenderThread()
{
    threadChecker.assertThread();

    renderManager.init(window);

    while (!engineShuttingDown.load())
    {
        renderManager.update();
    }

    renderManager.shutdown();
}

Entity Engine::ensureCamera()
{
    threadChecker.assertThread();

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
    threadChecker.assertThread();

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
}

bool Engine::update(float deltaTime)
{
    threadChecker.assertThread();

    if (shutdownRequested || Platform::Window::isWindowClosing(window))
    {
        shutdown();
        return false;
    }

    EngineSystems::update(world, player, deltaTime);

    Input::postUpdate();
    Platform::update();

    return true;
}

void Engine::shutdown()
{
    threadChecker.assertThread();

    if (engineShuttingDown.exchange(true))
        return;

    std::cout << "[Application] Shutting down...\n";

    EngineSystems::shutdown();

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

Entity Engine::getEntityUnderCursor()
{
    return Physics::lineTrace(world, Physics::rayFromViewportUV(world, player, Input::getCursorScreenPosition(window)), TraceChannelFlags::Default);
}

void Engine::openProject(std::filesystem::path path)
{
    threadChecker.assertThread();

    EngineSystems::reset();

    Project::open(std::move(path), world);

    player.setMainCamera(world, ensureCamera());
}

void Engine::saveCurrentProject()
{
    threadChecker.assertThread();

    Project::saveToCurrent(world);
}

Entity Engine::createEntity()
{
    threadChecker.assertThread();

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

bool Engine::hasComponent(Entity entity, TypeId componentTypeId)
{
    return world.hasComponent(entity, componentTypeId);
}

const ComponentBase& Engine::readComponent(Entity entity, TypeId componentType)
{
    return world.readComponent(entity, componentType);
}

ComponentBase& Engine::editComponent(Entity entity, TypeId componentType)
{
    return world.editComponent(entity, componentType);
}

void Engine::printArchetypeStatus()
{
    world.printArchetypeStatus();
}

EventBus& Engine::events()
{
    return eventBus;
}

EditorUIContext Engine::getEditorContext()
{
    return {.world = &world, .window = window };
}

void Engine::setEditorDrawCallback(std::function<void()> callback)
{
    renderManager.setEditorDrawCallback(std::move(callback));
}

void Engine::setViewportArea(Rect area)
{
    renderManager.setViewportArea(area);
}

Ray Engine::getViewportCursorRay(const World& world)
{
    const Vec2 cursor = Platform::Window::getCursorPosition(window);

    const auto [position, size] = renderManager.getViewportArea();

    const Vec2 uv {
        (cursor.x - position.x) / size.width,
        (cursor.y - position.y) / size.height
    };

    return Physics::rayFromViewportUV(world, player, uv);
}

Player& Engine::getPlayer()
{
    return player;
}

