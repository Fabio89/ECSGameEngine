export module Engine.DirtyTracker;
import Core;

export class DirtyTracker
{
public:
    void mark(Entity entity);

    [[nodiscard]]
    std::span<const Entity> getDirty() const;

    bool isDirty(Entity entity) const;

    void remove(Entity entity);

    void nextFrame();

private:
    UInt32 m_generation{1};
    std::vector<UInt32> m_marks;
    std::array<std::vector<Entity>, 2> m_dirtyBuffers;
    UInt32 m_currentBuffer{0};
    UInt32 m_nextBuffer{1};
};

export class DirtyTrackerManager
{
public:
    template<typename T>
    void markDirty(Entity entity);

    template<typename T>
    std::span<const Entity> getDirty();

    template<typename T>
    bool isDirty(Entity entity) const;

    void nextFrame();

    void remove(Entity entity);

private:
    template<typename T>
    DirtyTracker& getTracker();

    std::unordered_map<TypeId, DirtyTracker> m_dirtyTrackers;
};

void DirtyTracker::mark(Entity entity)
{
    if (!entity)
        return;

    if (entity.value >= m_marks.size())
        m_marks.resize(entity.value + 1);

    if (m_marks[entity.value] != m_generation)
    {
        m_marks[entity.value] = m_generation;
        m_dirtyBuffers[m_nextBuffer].push_back(entity);
    }
}

std::span<const Entity> DirtyTracker::getDirty() const
{
    return m_dirtyBuffers[m_currentBuffer];
}

bool DirtyTracker::isDirty(Entity entity) const
{
    return entity && entity.value < m_marks.size() && m_marks[entity.value] == m_generation;
}

void DirtyTracker::remove(Entity entity)
{
    std::erase(m_dirtyBuffers[m_nextBuffer], entity);
}

void DirtyTracker::nextFrame()
{
    std::swap(m_currentBuffer, m_nextBuffer);
    m_dirtyBuffers[m_nextBuffer].clear();

    ++m_generation;

    if (m_generation == 0)
    {
        m_generation = 1;
        std::ranges::fill(m_marks, 0);
    }
}

template<typename T>
void DirtyTrackerManager::markDirty(Entity entity)
{
    getTracker<T>().mark(entity);
}

template<typename T>
std::span<const Entity> DirtyTrackerManager::getDirty()
{
    return getTracker<T>().getDirty();
}

template<typename T>
bool DirtyTrackerManager::isDirty(Entity entity) const
{
    auto it = m_dirtyTrackers.find(getTypeId<T>());
    return it != m_dirtyTrackers.end() && it->second.isDirty(entity);
}

template<typename T>
DirtyTracker& DirtyTrackerManager::getTracker()
{
    auto it = m_dirtyTrackers.find(getTypeId<T>());
    if (it == m_dirtyTrackers.end())
        it = m_dirtyTrackers.try_emplace(getTypeId<T>(), DirtyTracker{}).first;
    return it->second;
}

void DirtyTrackerManager::nextFrame()
{
    for (DirtyTracker& tracker : m_dirtyTrackers | std::views::values)
        tracker.nextFrame();
}

void DirtyTrackerManager::remove(Entity entity)
{
    for (DirtyTracker& tracker : m_dirtyTrackers | std::views::values)
        tracker.remove(entity);
}
