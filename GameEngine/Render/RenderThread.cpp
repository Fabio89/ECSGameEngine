module Engine:Render.RenderThread;
import :Render.RenderManager;

void renderThreadFunc(const RenderThreadParams& params, const RenderThreadState& sharedState)
{
    try
    {
        auto& renderManager = *params.renderManager;
        renderManager.init(params.window);

        performLoop
        (
            params.settings,
            [&](float deltaTime) { renderManager.update(deltaTime); },
            [&] { return !closing.load(); }
        );

        std::cout << "[Application] Closing...\n";

        renderManager.shutdown();
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
}
