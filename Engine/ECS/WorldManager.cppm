export module Engine.WorldManager;
export import World;
import EventBus;

export class WorldManager
{
public:
    void shutdown();

    [[nodiscard]] WorldHandle createWorld();

    void destroyWorld(WorldHandle handle);

    [[nodiscard]] World& get(WorldHandle handle);

    template <typename Fn>
    void forEachWorld(Fn&& fn);

    [[nodiscard]] bool isValid(WorldHandle handle) const;

    template<typename Func> [[nodiscard]]
    EventBus::Subscription subscribe(Func&& callback);

    void nextFrame();

private:
    WorldHandle prepareWorld(UInt32 index);
    void subscribeToWorldEvents(World& world);

    struct WorldSlot
    {
        World world;
        UInt32 generation{};
        bool alive{false};
    };

    std::vector<std::unique_ptr<WorldSlot>> m_worlds;
    std::vector<UInt32> m_freeList;
    EventBus m_eventBus;
    EventSubscription m_subscription;
};

template<typename Fn>
void WorldManager::forEachWorld(Fn&& fn)
{
    for (auto& slot : m_worlds)
        if (slot->alive)
            fn(slot->world);
}

template<typename Func>
EventBus::Subscription WorldManager::subscribe(Func&& callback)
{
    return m_eventBus.subscribe(std::forward<Func>(callback));
}
