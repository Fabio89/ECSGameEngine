export module Engine:Application;
import :Config;
import :Render.RenderThread;
import :Component.Model;
import :Component.Transform;
import :DebugWidget.EntityExplorer;
import :World;

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

void runTest()
{
    const ApplicationSettings& settings = Config::getApplicationSettings();
    const LoopSettings loopSettings{.targetFps = settings.targetFps};

    RenderThread renderThread{{loopSettings}};
    World world{settings, renderThread.getRenderManager()};

    addSystems<ModelSystem, TransformSystem>(world);
    world.addDebugWidget<DebugWidgets::EntityExplorer>();
    world.addDebugWidget<DebugWidgets::ImGuiDemo>();

    world.createObjectsFromConfig();
    
    auto gameTick = [&world](float deltaTime)
    {
        world.updateSystems(deltaTime);
    };

    auto shouldRun = [&renderThread] { return !renderThread.isClosing(); };

    performLoop(loopSettings, gameTick, shouldRun);
}