module Engine;
import AssetLoader.Mesh;
import AssetManager;
import Engine.Config;
import Engine.FrameTimer;
import Engine.WorldManager;
import EngineSystems;
import Platform;
import Thread;

namespace
{
    bool shutdownRequested = false;
    std::atomic engineShuttingDown = false;
    std::thread renderThread;
    Player player;
    WindowHandle window;
    EventBus eventBus;
    ThreadOwned threadChecker;
    FrameTimer frameTimer;
    Engine::Config config;

    RenderManager renderManager;
    WorldManager worldManager;
}

namespace Engine
{
    void runRenderThread();
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

void Engine::init()
{
    threadChecker.assertThread();

    config = loadConfig();

    Platform::init();

    if (!check(!window.isValid(), "Can't create more than one window!"))
        return;

    window = Platform::Window::createWindow({
        .size = config.window.size,
        .mode = config.window.maximized ? WindowMode::Maximized : WindowMode::Windowed
    });

    Input::init(window);
    EngineComponents::init();

    AssetManager::registerLoader<MeshData>(std::make_unique<MeshAssetLoader>());
    AssetManager::registerLoader<TextureData>(std::make_unique<TextureAssetLoader>());
}

void Engine::start()
{
    threadChecker.assertThread();
    renderThread = std::thread{runRenderThread};
}

bool Engine::update()
{
    threadChecker.assertThread();

    frameTimer.waitForTarget(config.simulationHz);
    const float deltaTime = frameTimer.tick();

    if (shutdownRequested || Platform::Window::isWindowClosing(window))
    {
        shutdown();
        return false;
    }

    worldManager.forEachWorld([deltaTime](World& world)
    {
        EngineSystems::update(world, player, deltaTime);
    });

    Input::postUpdate(window);
    Platform::update();

    return true;
}

float Engine::getSimulationDeltaTime()
{
    return frameTimer.deltaTime();
}

float Engine::getRenderDeltaTime()
{
    return renderManager.getDeltaTime();
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

WindowHandle Engine::getWindow()
{
    return window;
}

WorldHandle Engine::createWorld()
{
    return worldManager.createWorld(renderManager);
}

World& Engine::getWorld(WorldHandle handle)
{
    return worldManager.get(handle);
}

EventBus& Engine::events()
{
    return eventBus;
}

ViewportId Engine::createViewport(WorldHandle world, Rect area)
{
    return renderManager.createViewport(world, area);
}

void Engine::setEditorCallbacks(EditorCallbacks callbacks)
{
    renderManager.setEditorCallbacks(std::move(callbacks));
}

void Engine::setViewportArea(ViewportId id, Rect area)
{
    renderManager.setViewportArea(id, area);
}

Ray Engine::getViewportCursorRay(const World& world)
{
    const Vec2 cursor = Platform::Window::getCursorPosition(window);

    const auto [position, size] = renderManager.getViewportArea();

    const Vec2 uv{
        (cursor.x - position.x) / size.width,
        (cursor.y - position.y) / size.height
    };

    return Physics::rayFromViewportUV(world, player, uv);
}

Player& Engine::getPlayer()
{
    return player;
}
