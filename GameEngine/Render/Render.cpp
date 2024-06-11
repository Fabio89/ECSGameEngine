module Engine.Render;
import "VulkanMain.h";

void renderThreadFunc(LoopSettings settings, ApplicationState& state)
{
	HelloTriangleApplication application;
	application.init();

	performLoop(settings, [&](float deltaTime) { application.update(); }, [&] {	return !application.shouldWindowClose(); });

	{
		std::lock_guard<std::mutex> lock(state.mutex);
		state.closing = true;
		std::cout << "[Application] Closing...\n";
	}

	application.shutdown();
	std::cout << "[Render] Shutdown complete.\n";
}

std::thread runRenderThread(LoopSettings settings, ApplicationState& state)
{
	return std::thread{ renderThreadFunc, LoopSettings{.targetFps = 144 }, std::ref(state) };
}
