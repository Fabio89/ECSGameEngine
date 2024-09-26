import Engine.ApplicationState;
import Engine.Config;
import Engine.Core;
import Engine.DebugWidget.EntityExplorer;
import Engine.Render;
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
    ApplicationState globalState;

    World world{settings, globalState};
    const LoopSettings loopSettings{.targetFps = settings.targetFps};

    std::thread renderThread = runRenderThread(loopSettings, globalState);

    while (!globalState.initialized)
    {
        //std::cout << "Waiting for render thread...\n";
    }

    gameInit(world);

    auto gameTick = [&world](float deltaTime)
    {
        world.updateSystems(deltaTime);
    };

    auto shouldRun = [&globalState] { return !globalState.closing; };

    performLoop(loopSettings, gameTick, shouldRun);

    gameShutdown();
    renderThread.join();

    return 0;
}
