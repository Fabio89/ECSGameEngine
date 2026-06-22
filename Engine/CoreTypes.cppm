export module CoreTypes;
import std;

export class DeltaTimeTracker
{
public:
    operator float() const { return getDeltaTime(); }
    float getDeltaTime() const { return m_deltaTime; }
    float update()
    {
        const auto currentTime = std::chrono::steady_clock::now();
        const std::chrono::duration<float> delta = currentTime - m_lastFrameTime;
        m_lastFrameTime = currentTime;
        return m_deltaTime = delta.count();
    }
    
private:
    std::chrono::steady_clock::time_point m_lastFrameTime;
    float m_deltaTime{};
};

export template <typename T>
class ThreadSafeQueue
{
public:
    void push(T item)
    {
        std::lock_guard lock{m_mutex};
        m_queue.emplace(std::move(item));
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
        std::lock_guard lock{m_mutex};
        m_condition.wait(lock, [this] { return !m_queue.empty(); });
        item = m_queue.front();
        m_queue.pop();
    }

    void clear()
    {
        std::lock_guard lock{m_mutex};
        m_queue = {};
    }

private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condition;
};
