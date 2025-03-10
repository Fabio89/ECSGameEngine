import Engine.RenderManager;
import Engine.Config;
import Engine.Core;
import Engine.DebugWidget.EntityExplorer;
import Engine.RenderThread;
import Engine.World;
import Engine.Component.Model;
import Engine.Component.Transform;
import std;

void gameInit(World& world)
{
    // Create systems
    world.addSystem<ModelSystem>();
    world.addSystem<TransformSystem>();

    world.createObjectsFromConfig();
    world.addDebugWidget<DebugWidgets::EntityExplorer>();
    world.addDebugWidget<DebugWidgets::ImGuiDemo>();
}


void gameShutdown()
{
    std::cout << "[Game] Shutdown complete.\n";
}

int main()
{
    const ApplicationSettings& settings = Config::getApplicationSettings();
    const LoopSettings loopSettings{.targetFps = settings.targetFps};

    RenderThread renderThread{{loopSettings}};
    World world{settings, renderThread.getRenderManager()};

    gameInit(world);

    auto gameTick = [&world](float deltaTime)
    {
        world.updateSystems(deltaTime);
    };

    auto shouldRun = [&renderThread] { return !renderThread.isClosing(); };

    performLoop(loopSettings, gameTick, shouldRun);

    gameShutdown();

    return 0;
}
