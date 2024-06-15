export module Engine.Render;
import std;
import <windows.h>;

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
void performLoop(const LoopSettings& settings, T_Body&& body, T_Condition&& condition)
{
	const auto targetFrameDuration = std::chrono::microseconds{ static_cast<int>(1000000 / settings.targetFps) };
	std::chrono::microseconds adjustment{ 0 };

	while (condition())
	{
		const auto frameStart = std::chrono::steady_clock::now();

		body(targetFrameDuration.count() / 1000000.f);

		//std::cout << adjustment.count() << std::endl;

		const auto frameEnd = std::chrono::steady_clock::now();
		const auto frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - frameStart);
		if (frameDuration < targetFrameDuration && settings.targetFps > 0.f)
		{
			std::chrono::microseconds sleepTime = targetFrameDuration - frameDuration - adjustment;
			if (sleepTime > std::chrono::milliseconds(0))
			{
				timeBeginPeriod(1);
				std::this_thread::sleep_for(sleepTime);
				timeEndPeriod(1);
			}

			auto finalFrameDuration = std::chrono::steady_clock::now() - frameStart;
			std::chrono::microseconds extraTimeElapsed = std::chrono::duration_cast<std::chrono::microseconds>(finalFrameDuration - targetFrameDuration);
			adjustment = max(std::chrono::microseconds{ 0 }, extraTimeElapsed + std::chrono::milliseconds{ 1 });
		}
	}
}