export module Engine.DirtyTracker;
import Core;

class ComponentDirtyTrackerBase
{
public:
    virtual ~ComponentDirtyTrackerBase() = default;

    virtual void nextFrame() = 0;
};

template<typename T>
class ComponentDirtyTracker : public ComponentDirtyTrackerBase
{
public:
    void mark(Entity entity);

    [[nodiscard]]
    std::span<const Entity> dirty() const;

    void nextFrame() override;

private:
    UInt32 m_generation{1};
    std::vector<UInt32> m_marks;
    std::vector<Entity> m_dirtyEntities;
};

export class DirtyTracker
{
public:
    template<typename T>
    void markDirty(Entity entity);

    template<typename T>
    std::span<const Entity> dirty();

    void nextFrame();

private:
    template<typename T>
    ComponentDirtyTracker<T>& getTracker()
    {
        auto it = m_dirtyTrackers.find(getTypeId<T>());
        if (it == m_dirtyTrackers.end())
            it = m_dirtyTrackers.try_emplace(getTypeId<T>(), std::make_unique<ComponentDirtyTracker<T>>()).first;
        return static_cast<ComponentDirtyTracker<T>&>(*it->second.get());
    }

    std::unordered_map<TypeId, std::unique_ptr<ComponentDirtyTrackerBase>> m_dirtyTrackers;
};

template<typename T>
void ComponentDirtyTracker<T>::mark(Entity entity)
{
    if (!entity)
        return;

    if (entity.value >= m_marks.size())
        m_marks.resize(entity.value + 1);

    if (m_marks[entity.value] != m_generation)
    {
        m_marks[entity.value] = m_generation;
        m_dirtyEntities.push_back(entity);
    }
}

template<typename T>
std::span<const Entity> ComponentDirtyTracker<T>::dirty() const
{
    return m_dirtyEntities;
}

template<typename T>
void ComponentDirtyTracker<T>::nextFrame()
{
    m_dirtyEntities.clear();

    ++m_generation;

    if (m_generation == 0)
    {
        m_generation = 1;
        std::ranges::fill(m_marks, 0);
    }
}

template<typename T>
void DirtyTracker::markDirty(Entity entity)
{
    getTracker<T>().mark(entity);
}

template<typename T>
std::span<const Entity> DirtyTracker::dirty()
{
    return getTracker<T>().dirty();
}

void DirtyTracker::nextFrame()
{
    for (std::unique_ptr<ComponentDirtyTrackerBase>& tracker : m_dirtyTrackers | std::views::values)
        tracker->nextFrame();
}
