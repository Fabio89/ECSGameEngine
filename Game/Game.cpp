import Engine;
import std;
import <chrono>;

int main()
{
    auto window = createViewportWindow(nullptr, 800, 600);
    engineInit(window);
    bool running{true};
    constexpr std::chrono::milliseconds deltaTime{8};
    while (running)
    {
        running = engineUpdate(window, static_cast<float>(deltaTime.count()));
        std::this_thread::sleep_for(deltaTime);
    }
    engineShutdown(window);
    return 0;
}