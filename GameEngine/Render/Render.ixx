module;

#pragma warning(disable : 5105)
#include <windows.h>

export module Engine.Render;
import Engine.Core;
import std;

export struct LoopSettings
{
	float targetFps = 120.f;
};

class VulkanApplication;

export struct ApplicationState
{
	std::mutex mutex;
	VulkanApplication* application{nullptr};
	bool initialized = false;
	bool closing = false;
};

export std::thread runRenderThread(LoopSettings settings, ApplicationState& state);

export template<typename T_Body, typename T_Condition>
void performLoop(const LoopSettings& settings, T_Body&& body, T_Condition&& condition)
{
	using ms = std::chrono::milliseconds;
	const auto targetFrameDuration = ms{ static_cast<int>(1000 / settings.targetFps) };
	auto lastFrameDuration = targetFrameDuration;
	ms adjustment{ 0 };

	while (condition())
	{
		const auto frameStart = std::chrono::steady_clock::now();

		body(lastFrameDuration.count() / 1000.f);

		//std::cout << adjustment.count() << std::endl;

		const auto frameEnd = std::chrono::steady_clock::now();
		const auto frameDuration = std::chrono::duration_cast<ms>(frameEnd - frameStart);
		if (frameDuration < targetFrameDuration && settings.targetFps > 0.f)
		{
			ms sleepTime = targetFrameDuration - frameDuration - adjustment;
			if (sleepTime > ms(0))
			{
				timeBeginPeriod(1);
				std::this_thread::sleep_for(sleepTime);
				timeEndPeriod(1);
			}

			lastFrameDuration = std::chrono::duration_cast<ms>(std::chrono::steady_clock::now() - frameStart);
			ms extraTimeElapsed = std::chrono::duration_cast<ms>(lastFrameDuration - targetFrameDuration);
			adjustment = max(ms{ 0 }, extraTimeElapsed + ms{ 1 });
		}
	}
}