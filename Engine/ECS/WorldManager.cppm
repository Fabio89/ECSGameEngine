export module Engine.WorldManager;
import Render.RenderManager;
import World;
import WorldHandle;
import EngineSystems;

struct WorldSlot
{
    World world;
    UInt32 generation{};
    bool alive{false};
};

export class WorldManager
{
public:
    WorldHandle createWorld(RenderManager& renderManager)
    {
        if (!m_freeList.empty())
        {
            const UInt32 index = m_freeList.back();
            m_freeList.pop_back();

            return prepareWorld(index, renderManager);
        }

        const UInt32 index = static_cast<UInt32>(m_worlds.size());
        m_worlds.emplace_back();

        return prepareWorld(index, renderManager);
    }

    void destroyWorld(WorldHandle handle)
    {
        if (!isValid(handle))
            return;

        WorldSlot& slot = m_worlds[handle.index];

        slot.world = World{};
        slot.alive = false;
        slot.generation++;

        m_freeList.push_back(handle.index);
    }

    World& get(WorldHandle handle)
    {
        check(isValid(handle), "");
        return m_worlds[handle.index].world;
    }

    template <typename Fn>
    void forEachWorld(Fn&& fn)
    {
        for (auto& slot : m_worlds)
        {
            if (slot.alive)
                fn(slot.world);
        }
    }

    bool isValid(WorldHandle handle) const
    {
        if (handle.index >= m_worlds.size())
            return false;

        const WorldSlot& slot = m_worlds[handle.index];

        return slot.alive && slot.generation == handle.generation;
    }

private:
    WorldHandle prepareWorld(UInt32 index, RenderManager& renderManager)
    {
        WorldSlot& slot = m_worlds[index];
        const WorldHandle handle{index, slot.generation};
        slot.alive = true;
        slot.world = World{{handle, &renderManager}};
        EngineSystems::init(slot.world);
        return handle;
    }

    std::vector<WorldSlot> m_worlds;
    std::vector<UInt32> m_freeList;
};