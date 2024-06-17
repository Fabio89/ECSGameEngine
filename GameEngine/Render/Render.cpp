module Engine.Render;
import Engine.Render.Application;

void renderThreadFunc(LoopSettings settings, ApplicationState& state)
{
	try
	{
		VulkanApplication application;
		application.init();

		performLoop(settings, [&](float deltaTime) { application.update(deltaTime); }, [&] {	return !application.shouldWindowClose(); });

		{
			std::lock_guard lock(state.mutex);
			state.closing = true;
			std::cout << "[Application] Closing...\n";
		}

		application.shutdown();
		std::cout << "[Render] Shutdown complete.\n";
	}
	catch (const std::exception& ex)
	{
		std::cerr << "Error: " << ex.what() << std::endl;
	}
}

std::thread runRenderThread(LoopSettings settings, ApplicationState& state)
{
	return std::thread{ renderThreadFunc, settings, std::ref(state) };
}
