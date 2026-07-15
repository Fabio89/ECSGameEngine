module Engine.WorldManager;
import EventBus;
import World.Events;

void WorldManager::shutdown()
{
    m_subscription.clear();
}

WorldHandle WorldManager::createWorld()
{
    if (!m_freeList.empty())
    {
        const UInt32 index = m_freeList.back();
        m_freeList.pop_back();

        return prepareWorld(index);
    }

    const UInt32 index = static_cast<UInt32>(m_worlds.size());
    m_worlds.emplace_back(std::make_unique<WorldSlot>());

    return prepareWorld(index);
}

void WorldManager::destroyWorld(WorldHandle handle)
{
    if (!isValid(handle))
        return;

    WorldSlot& slot = *m_worlds[handle.index];

    slot.world = World{};
    slot.alive = false;
    slot.generation++;

    m_eventBus.publish(WorldEvents::WorldDestroyed{handle});

    m_freeList.push_back(handle.index);
}

World& WorldManager::get(WorldHandle handle)
{
    check(isValid(handle), "");
    return m_worlds[handle.index]->world;
}

bool WorldManager::isValid(WorldHandle handle) const
{
    if (handle.index >= m_worlds.size())
        return false;

    const WorldSlot& slot = *m_worlds[handle.index];
    return slot.alive && slot.generation == handle.generation;
}

void WorldManager::nextFrame()
{
    for (auto& slot : m_worlds)
        slot->world.nextFrame();
}

WorldHandle WorldManager::prepareWorld(UInt32 index)
{
    WorldSlot& slot = *m_worlds[index];
    const WorldHandle handle{index, slot.generation};
    slot.alive = true;
    slot.world = World{{handle}};

    m_eventBus.publish(WorldEvents::WorldCreated{handle});

    subscribeToWorldEvents(slot.world);

    return handle;
}

void WorldManager::subscribeToWorldEvents(World& world)
{
    m_subscription += world.subscribe([this](const WorldEvents::ComponentAdded& event)
    {
        m_eventBus.publish(event);
    });

    m_subscription += world.subscribe([this](const WorldEvents::ComponentRemoved& event)
    {
        m_eventBus.publish(event);
    });

    m_subscription += world.subscribe([this](const WorldEvents::EntityCreated& event)
    {
        m_eventBus.publish(event);
    });

    m_subscription += world.subscribe([this](const WorldEvents::EntityDestroyed& event)
    {
        m_eventBus.publish(event);
    });

    m_subscription += world.subscribe([this](const WorldEvents::SceneLoaded& event)
    {
        m_eventBus.publish(event);
    });

    m_subscription += world.subscribe([this](const WorldEvents::SceneUnloaded& event)
    {
        m_eventBus.publish(event);
    });
}
