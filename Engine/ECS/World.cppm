module;

#include <EngineExport.h>

export module World;
export import Core;
export import Engine.ComponentAccess;
export import WorldHandle;
import Archetype;
import Engine.DirtyTracker;
import EventBus;
import Guid;
import Serialization.Json;
import Thread;
import World.Events;

constexpr int maxComponentsPerEntity = 64;

struct EntitySignature
{
    friend bool operator==(const EntitySignature& a, const EntitySignature& b) { return a.bitset == b.bitset; }
    std::bitset<maxComponentsPerEntity> bitset{};
};

template <>
struct std::hash<EntitySignature>
{
    std::size_t operator()(const EntitySignature& a) const noexcept { return hash<bitset<maxComponentsPerEntity>>()(a.bitset); }
};

export struct WorldCreateInfo
{
    WorldHandle handle{};
};

//------------------------------------------------------------------------------------------------------------------------
// Query
//------------------------------------------------------------------------------------------------------------------------

export class World;

template<bool Const, typename... Access>
concept ValidQueryAccess = !Const || (... && std::same_as<typename AccessTraits<Access>::AccessType, const typename AccessTraits<Access>::ComponentType&>);

template<bool Const, typename... Access> requires ValidQueryAccess<Const, Access...>
class QueryImpl
{
public:
    static_assert(!(Const && (... || AccessTraits<Access>::writable)), "Edit<T> cannot be used with a const World query.");

    using Components = std::tuple<typename AccessTraits<Access>::ComponentType...>;
    using ArchetypeType = std::conditional_t<Const, const Archetype, Archetype>;
    using WorldType = std::conditional_t<Const, const World, World>;

    struct Iterator
    {
        Iterator(WorldType& world, const std::vector<ArchetypeType*>& archetypes, std::size_t archetypeIndex, Int32 entityIndex)
            : m_world{world},
              m_archetypes{archetypes},
              m_archetypeIndex{archetypeIndex},
              m_entityIndex{entityIndex}
        {
            skipEmptyArchetypes();
        }

        Iterator& operator++();

        auto operator*() const;

        bool operator==(const Iterator& other) const;

    private:
        ArchetypeType& getCurrentArchetype() const { return *m_archetypes[m_archetypeIndex]; }

        void skipEmptyArchetypes();

        template<typename AccessSpec>
        decltype(auto) makeAccess(Entity entity) const;

        WorldType& m_world;
        const std::vector<ArchetypeType*>& m_archetypes;
        std::size_t m_archetypeIndex{};
        Int32 m_entityIndex{};
    };

    explicit QueryImpl(WorldType& world);

    Iterator begin();

    Iterator end();

private:
    WorldType& m_world;
    std::vector<ArchetypeType*> m_archetypes;
};

export template<typename... Access>
using Query = QueryImpl<false, Access...>;

export template<typename... Access>
using ConstQuery = QueryImpl<true, Access...>;

//------------------------------------------------------------------------------------------------------------------------
// World
//------------------------------------------------------------------------------------------------------------------------
export class ENGINE_API World : ThreadOwned
{
public:
    World() = default;
    explicit World(const WorldCreateInfo&);
    World(const World&) = delete;
    World& operator=(const World&) = delete;
    World(World&&) noexcept = default;
    World& operator=(World&&) noexcept = default;

    [[nodiscard]] WorldHandle getHandle() const { return m_handle; }
    Entity createEntity();
    void removeEntity(Entity entity);
    bool isValid(Entity entity) const;

    void loadScene(const std::filesystem::path& path);
    void unloadScene();
    JsonObject serializeScene(Json::MemoryPoolAllocator<>& allocator) const;
    void deserializeScene(const JsonObject& json);

    template <ValidComponentData T, typename... Args>
    const T& addComponent(Entity entity, Args&&... args);

    template <ValidComponentData T>
    const T& addComponent(Entity entity, T&& args);

    template <ValidComponentData T>
    bool hasComponent(Entity entity) const;
    bool hasComponent(Entity entity, TypeId componentTypeId) const;

    Archetype::ComponentRange getComponentTypesInEntity(Entity entity) const;

    template<ValidComponentData T>
    const T& readComponent(Entity entity) const { return getComponent<T>(entity); }
    const ComponentBase& readComponent(Entity entity, TypeId componentType) const { return getComponent(entity, componentType); }

    template<ValidComponentData T>
    Edit<T> editComponent(Entity entity) { return Edit<T>{entity, getComponent<T>(entity), m_dirtyTracker}; }
    BaseEdit editComponent(Entity entity, TypeId componentType) { return BaseEdit{entity, const_cast<ComponentBase&>(getComponent(entity, componentType)), m_dirtyTracker}; }

    template<typename Func> [[nodiscard]]
    EventBus::Subscription subscribe(Func&& callback);

    auto getEntitiesRange() const { return m_entities | std::views::keys; }

    template<typename... Access>
    auto query() { return Query<Access...>{*this}; }

    template<typename... Access>
    auto query() const { return ConstQuery<Access...>{*this}; }

    void nextFrame();

    template<typename Tag>
    std::span<const Entity> getMarked() { return m_dirtyTracker.getDirty<Tag>(); }

    template<typename Tag>
    bool isMarked(Entity entity) const { return m_dirtyTracker.isDirty<Tag>(entity); }

    template<typename Tag>
    void mark(Entity entity) { m_dirtyTracker.markDirty<Tag>(entity); }

    auto archetypes() { return m_archetypes | std::views::values; }
    auto archetypes() const { return m_archetypes | std::views::values; }
    void printArchetypeStatus();

private:
    const Archetype& readArchetype(const EntitySignature& signature) const;
    Archetype& editArchetype(const EntitySignature& signature);
    Archetype& editOrCreateArchetype(const EntitySignature& signature);
    Archetype& prepareArchetypeOnAddComponent(Entity entity, TypeId componentId);

    template<ValidComponentData T>
    const T& getComponent(Entity entity) const;

    template<ValidComponentData T>
    T& getComponent(Entity entity);

    const ComponentBase& getComponent(Entity entity, TypeId componentType) const;

    WorldHandle m_handle;
    Entity::ValueType m_nextEntityValue{};
    std::unordered_map<Entity, EntitySignature> m_entities{};
    std::unordered_map<EntitySignature, Archetype> m_archetypes;

    DirtyTrackerManager m_dirtyTracker;

    EventBus m_eventBus;
};

//------------------------------------------------------------------------------------------------------------------------
// Query - Implementation
//------------------------------------------------------------------------------------------------------------------------

template<bool Const, typename... Access> requires ValidQueryAccess<Const, Access...>
QueryImpl<Const, Access...>::Iterator& QueryImpl<Const, Access...>::Iterator::operator++()
{
    ++m_entityIndex;

    if (m_entityIndex >= getCurrentArchetype().getSize())
    {
        m_entityIndex = 0;
        ++m_archetypeIndex;

        skipEmptyArchetypes();
    }

    return *this;
}

template<bool Const, typename... Access> requires ValidQueryAccess<Const, Access...>
auto QueryImpl<Const, Access...>::Iterator::operator*() const
{
    const Entity entity = getCurrentArchetype().getEntityAt(m_entityIndex);
    return std::tuple<Entity, typename AccessTraits<Access>::AccessType...>(entity, makeAccess<Access>(entity)...);
}

template<bool Const, typename... Access> requires ValidQueryAccess<Const, Access...>
bool QueryImpl<Const, Access...>::Iterator::operator==(const Iterator& other) const
{
    return m_archetypeIndex == other.m_archetypeIndex && m_entityIndex == other.m_entityIndex;
}

template<bool Const, typename... Access> requires ValidQueryAccess<Const, Access...>
void QueryImpl<Const, Access...>::Iterator::skipEmptyArchetypes()
{
    while (m_archetypeIndex < m_archetypes.size())
    {
        if (getCurrentArchetype().getSize() > 0)
            return;

        ++m_archetypeIndex;
    }
}

template<bool Const, typename... Access> requires ValidQueryAccess<Const, Access...>
template<typename AccessSpec>
decltype(auto) QueryImpl<Const, Access...>::Iterator::makeAccess(Entity entity) const
{
    using T = AccessTraits<AccessSpec>::ComponentType;

    if constexpr (std::same_as<AccessSpec, Edit<T>>)
        return m_world.template editComponent<T>(entity);
    else
        return m_world.template readComponent<T>(entity);
}

template<bool Const, typename... Access> requires ValidQueryAccess<Const, Access...>
QueryImpl<Const, Access...>::QueryImpl(WorldType& world) : m_world{world}
{
    for (ArchetypeType& archetype : m_world.archetypes())
    {
        if (archetype.template matches<typename AccessTraits<Access>::ComponentType...>())
        {
            m_archetypes.push_back(&archetype);
        }
    }
}

template<bool Const, typename... Access> requires ValidQueryAccess<Const, Access...>
QueryImpl<Const, Access...>::Iterator QueryImpl<Const, Access...>::begin()
{
    return Iterator{m_world, m_archetypes, 0, 0};
}

template<bool Const, typename... Access> requires ValidQueryAccess<Const, Access...>
QueryImpl<Const, Access...>::Iterator QueryImpl<Const, Access...>::end()
{
    return Iterator{m_world, m_archetypes, m_archetypes.size(), 0};
}

//------------------------------------------------------------------------------------------------------------------------
// World - Implementation
//------------------------------------------------------------------------------------------------------------------------

template<ValidComponentData T, typename... Args>
const T& World::addComponent(Entity entity, Args&&... args)
{
    assertThread();
    Archetype& archetype = prepareArchetypeOnAddComponent(entity, getTypeId<T>());
    T& addedComponent = archetype.addComponent<T>(entity, T{std::forward<Args>(args)...});
    log(std::format("Added component {} to entity {}", getTypeName<T>(), entity));
    m_dirtyTracker.markDirty<T>(entity);
    m_eventBus.publish(WorldEvents::ComponentAdded{.world = getHandle(), .entity = entity, .componentType = getTypeId<T>()});

    return addedComponent;
}

template <ValidComponentData T>
const T& World::addComponent(Entity entity, T&& args)
{
    return addComponent<T, T>(entity, std::forward<T>(args));
}

template <ValidComponentData T>
bool World::hasComponent(Entity entity) const
{
    if (auto it = m_entities.find(entity); it != m_entities.end())
    {
        return readArchetype(it->second).matches<T>();
    }
    report(std::format("{} was requested for entity {} which doesn't exist", getTypeName<T>(), entity));
    return false;
}

template<typename Func>
EventBus::Subscription World::subscribe(Func&& callback)
{
    return m_eventBus.subscribe(std::forward<Func>(callback));
}

template <ValidComponentData T>
const T& World::getComponent(Entity entity) const
{
    if (auto it = m_entities.find(entity); it != m_entities.end())
    {
        return readArchetype(it->second).readComponent<T>(entity);
    }
    fatalError(std::format("Couldn't find component: {}", getTypeName<T>()));
    static const T invalid{};
    return invalid;
}

template<ValidComponentData T>
T& World::getComponent(Entity entity)
{
    return const_cast<T&>(std::as_const(*this).getComponent<T>(entity));
}