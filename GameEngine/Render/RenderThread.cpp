module Engine.RenderThread;
import Engine.Render.Application;
import Engine.RenderManager;

void renderThreadFunc(RenderThreadParams params, RenderThreadState& sharedState)
{
    try
    {
        VulkanApplication application;
        application.init();
        sharedState.renderManager = &application;
        sharedState.initialised = true;
        sharedState.initialisedCondition.notify_all();

        performLoop
        (
            params.settings,
            [&](float deltaTime) { application.update(deltaTime); },
            [&] { return !application.shouldWindowClose(); }
        );

        sharedState.closing = true;
        std::cout << "[Application] Closing...\n";

        application.shutdown();
        std::cout << "[Render] Shutdown complete.\n";
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        throw;
    }
}

RenderThread::RenderThread(RenderThreadParams params)
    : m_thread{std::thread{renderThreadFunc, params, std::ref(m_sharedState)}}
{
    waitTillInitialised();
}

void RenderThread::waitTillInitialised()
{
    const auto start = std::chrono::high_resolution_clock::now();

    std::mutex mutex;
    std::unique_lock lock{mutex};
    m_sharedState.initialisedCondition.wait(lock, [this] { return m_sharedState.initialised.load(); });

    const std::chrono::duration<double, std::milli> duration = std::chrono::high_resolution_clock::now() - start;

    std::cout << "[Render] Initialised in " << duration.count() << "ms\n";
}
