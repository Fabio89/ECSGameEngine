export module Engine:Render.RenderThread;
import :Core;
import :Render.Vulkan;
import std;

export struct IRenderManager;

export struct LoopSettings
{
    float targetFps = 120.f;
};

export template <typename T>
class ThreadSafeQueue
{
public:
    void push(const T& item)
    {
        std::lock_guard lock{m_mutex};
        m_queue.push(item);
        m_condition.notify_one();
    }

    std::optional<T> tryPop()
    {
        std::optional<T> item;
        std::lock_guard lock{m_mutex};
        if (!m_queue.empty())
        {
            item.emplace(std::move(m_queue.front()));
            m_queue.pop();
        }
        return item;
    }

    void waitAndPop(T& item)
    {
        std::unique_lock lock{m_mutex};
        m_condition.wait(lock, [this] { return !m_queue.empty(); });
        item = m_queue.front();
        m_queue.pop();
    }

private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condition;
};

struct RenderThreadState
{
    //std::atomic<bool> closing{false};
};
std::atomic<bool> closing{false};
export struct RenderThreadParams
{
    IRenderManager* renderManager{};
    GLFWwindow* window{};
    LoopSettings settings;
};


export class RenderThread
{
public:
    explicit RenderThread(RenderThreadParams params);
    ~RenderThread() { Close(); }
    RenderThread(const RenderThread&) = delete;
    RenderThread(RenderThread&&) = delete;
    RenderThread& operator=(const RenderThread&) = delete;
    RenderThread& operator=(RenderThread&&) = delete;

    void Close()
    {
        if (m_thread.joinable())
        {
            closing = true;
            m_thread.join();
        }
    }

private:
    RenderThreadState m_sharedState;
    std::thread m_thread;
};

export template <typename T_Body, typename T_Condition>
void performLoop(const LoopSettings& settings, const T_Body& body, const T_Condition& condition)
{
    using ms = std::chrono::milliseconds;
    const auto targetFrameDuration = ms{static_cast<int>(1000 / settings.targetFps)};
    auto lastFrameDuration = targetFrameDuration;
    ms adjustment{0};

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
                std::this_thread::sleep_for(sleepTime);
            }

            lastFrameDuration = std::chrono::duration_cast<ms>(std::chrono::steady_clock::now() - frameStart);
            ms extraTimeElapsed = std::chrono::duration_cast<ms>(lastFrameDuration - targetFrameDuration);
            adjustment = max(ms{ 0 }, extraTimeElapsed + ms{ 1 });
        }
    }
}
