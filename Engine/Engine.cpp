module Engine;
import AssetLoader.Mesh;
import AssetManager;
import Engine.Config;
import Engine.FrameTimer;
import Input;
import Platform;
import Render.RenderManager;
import Thread;

namespace
{
    bool initialized = false;
    bool shutdownRequested = false;
    std::atomic engineShuttingDown = false;
    std::thread renderThread;
    WindowHandle window;
    ThreadOwned threadChecker;
    FrameTimer frameTimer;
    Engine::Config config;
    UInt64 currentFrame{};
    RenderManager renderManager;
    WorldManager worldManager;
    SystemManager systemManager{{.worlds = worldManager, .viewports = renderManager.viewports(), .renderCommands = renderManager.getCommandQueue()}};
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

    systemManager.init();
    initialized = true;
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

    systemManager.update(deltaTime);

    Input::postUpdate(window);
    Platform::update();

    worldManager.nextFrame();
    ++currentFrame;

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

    systemManager.shutdown();
    worldManager.shutdown();

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

UInt64 Engine::getCurrentFrame()
{
    return currentFrame;
}

WorldHandle Engine::createWorld()
{
    return worldManager.createWorld();
}

World& Engine::getWorld(WorldHandle handle)
{
    return worldManager.get(handle);
}

WorldManager& Engine::worlds()
{
    return worldManager;
}

void Engine::addSystem(SystemCallbacks callbacks)
{
    systemManager.add(std::move(callbacks));
}

ViewportId Engine::createViewport(std::vector<WorldHandle> worlds, Rect area)
{
    return renderManager.createViewport(worlds, area);
}

void Engine::setEditorCallbacks(EditorCallbacks callbacks)
{
    renderManager.setEditorCallbacks(std::move(callbacks));
}

void Engine::setViewportArea(ViewportId id, Rect area)
{
    renderManager.viewports().setViewportArea(id, area);
}

Rect Engine::getViewportArea(ViewportId id)
{
    return renderManager.viewports().getViewportArea(id);
}

Ray Engine::getViewportCursorRay(ViewportId id)
{
    const Vec2 cursor = Platform::Window::getCursorPosition(window);

    const auto [position, size] = renderManager.viewports().getViewportArea(id);

    const Vec2 uv{
        (cursor.x - position.x) / size.width,
        (cursor.y - position.y) / size.height
    };

    return Physics::rayFromViewportUV(renderManager.viewports().getCamera(id), uv);
}

ViewportManager& Engine::getViewportManager()
{
    return renderManager.viewports();
}

float Engine::getViewportAspectRatio(ViewportId id)
{
    return renderManager.viewports().getAspectRatio(id);
}

RenderCommandQueue& Engine::getRenderCommandQueue()
{
    return renderManager.getCommandQueue();
}