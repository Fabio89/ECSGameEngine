export module Editor.SnapshotFrame;

import Core;
import std;

export namespace Editor
{
    class SnapshotFrame
    {
    public:
        SnapshotFrame() = default;

        void clear()
        {
            m_data.clear();
        }

        template<typename T>
        void set(T value)
        {
            const auto id = getTypeId<T>();
            m_data[id] = std::move(value);
        }

        template<typename T>
        bool contains() const
        {
            const auto id = getTypeId<T>();
            return m_data.contains(id);
        }

        template<typename T>
        const T& get() const
        {
            return std::any_cast<const T&>(m_data.at(getTypeId<T>()));
        }

    private:
        std::unordered_map<TypeId, std::any> m_data;
    };

    class SnapshotPublisher
    {
    public:
        SnapshotPublisher() = default;
        SnapshotPublisher(SnapshotPublisher&&) noexcept;
        SnapshotPublisher& operator=(SnapshotPublisher&&) noexcept;

        SnapshotFrame& beginWrite()
        {
            SnapshotFrame& frame = m_frames[m_writeIndex];

            frame.clear();

            return frame;
        }

        void publish()
        {
            m_readIndex.store(m_writeIndex, std::memory_order_release);
            m_writeIndex ^= 1;
        }

        const SnapshotFrame& frame() const
        {
            return m_frames[m_readIndex.load(std::memory_order_acquire)];
        }

    private:

        std::array<SnapshotFrame, 2> m_frames{};
        std::atomic_uint32_t m_readIndex{0};
        UInt32 m_writeIndex{1};
    };
}
