export module Engine.Render;

import <mutex>;

export struct LoopSettings
{
	float targetFps = 120.f;
};

export struct ApplicationState
{
	std::mutex mutex;
	bool closing = false;
};

export std::thread runRenderThread(LoopSettings settings, ApplicationState& state);

export template<typename T_Body, typename T_Condition>
void PerformLoop(const LoopSettings& settings, T_Body&& body, T_Condition&& condition)
{
	const auto targetFrameDuration = std::chrono::milliseconds{ static_cast<int>(1000 / settings.targetFps) };

	while (condition())
	{
		const auto frameStart = std::chrono::high_resolution_clock::now();

		body(targetFrameDuration.count() / 1000.f);

		const auto frameEnd = std::chrono::high_resolution_clock::now();
		const auto frameDuration = std::chrono::duration_cast<std::chrono::milliseconds>(frameEnd - frameStart);

		if (frameDuration < targetFrameDuration)
		{
			std::this_thread::sleep_for(targetFrameDuration - frameDuration);
		}
	}
}