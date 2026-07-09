export module ThreadSafeQueue;
import std;

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

    bool tryPop(T& item)
    {
        std::lock_guard lock{m_mutex};

        if (m_queue.empty())
            return false;

        item = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }

    void waitPop(T& item)
    {
        std::unique_lock lock{m_mutex};
        m_condition.wait(lock, [this] { return !m_queue.empty(); });
        item = std::move(m_queue.front());
        m_queue.pop();
    }

    void clear()
    {
        std::lock_guard lock{m_mutex};
        m_queue = {};
    }

private:
    std::queue<T> m_queue{};
    std::mutex m_mutex{};
    std::condition_variable m_condition{};
};
