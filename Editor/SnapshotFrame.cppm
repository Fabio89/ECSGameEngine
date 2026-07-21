export module Editor.SnapshotFrame;

import Core;
import std;

export namespace Editor
{
    class SnapshotFrame
    {
    public:
        using Ptr = std::shared_ptr<SnapshotFrame>;
        using ConstPtr = std::shared_ptr<const SnapshotFrame>;

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

    class SnapshotPublisher : NoCopy
    {
    public:
        SnapshotFrame::Ptr beginWrite()
        {
            return std::make_shared<SnapshotFrame>();
        }

        void publish(SnapshotFrame::Ptr frame)
        {
            m_current.store(std::move(frame), std::memory_order_release);
        }

        SnapshotFrame::ConstPtr frame() const
        {
            return m_current.load(std::memory_order_acquire);
        }

    private:
        std::atomic<SnapshotFrame::ConstPtr> m_current{nullptr};
    };
}

export using SnapshotFramePtr = std::shared_ptr<Editor::SnapshotFrame>;
export using SnapshotFrameConstPtr = std::shared_ptr<const Editor::SnapshotFrame>;
export using Editor::SnapshotFrame;