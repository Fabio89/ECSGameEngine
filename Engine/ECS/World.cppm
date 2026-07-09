module;

#include <EngineExport.h>

export module World;
export import Core;
export import WorldHandle;
import Archetype;
import Guid;
import Render.RenderManager;
import Serialization.Json;
import Thread;

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
    RenderManager* renderManager{};
};

export using SystemCallbackHandle = int;

export struct SystemCallback
{
    ArchetypeChangedCallback onComponentAdded;
    std::function<void(Entity)> onEntityRemoved;
};

SystemCallbackHandle generateSystemCallbackHandle()
{
    static SystemCallbackHandle lastValue{-1};
    return ++lastValue;
}

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

    void patchEntity(Entity entity, const JsonObject& json);

    template <ValidComponentData T, typename... Args>
    T& addComponent(Entity entity, Args&&... args);

    template <ValidComponentData T>
    T& addComponent(Entity entity, T&& args);

    template <ValidComponentData T>
    bool hasComponent(Entity entity) const;
    bool hasComponent(Entity entity, TypeId componentTypeId) const;

    template <ValidComponentData T>
    const T& readComponent(Entity entity) const;

    const ComponentBase& readComponent(Entity entity, TypeId componentType) const;

    Archetype::ComponentRange getComponentTypesInEntity(Entity entity) const;

    template <ValidComponentData T>
    T& editComponent(Entity entity) { return const_cast<T&>(readComponent<T>(entity)); }

    ComponentBase& editComponent(Entity entity, TypeId componentType) { return const_cast<ComponentBase&>(readComponent(entity, componentType)); }

    SystemCallbackHandle registerSystem(SystemCallback callback);
    void unregisterSystem(SystemCallbackHandle handle);

    auto getEntitiesRange() const { return m_entities | std::views::keys; }

    template <ValidComponentData First, ValidComponentData ... Rest>
    std::generator<std::tuple<Entity, const First&, const Rest&...>> view() const;

    template <ValidComponentData First, ValidComponentData ... Rest>
    std::generator<std::tuple<Entity, First&, Rest&...>> view();

    const RenderManager& getRenderManager() const { return *m_renderManager; }
    RenderManager& getRenderManager() { return *m_renderManager; }

    void printArchetypeStatus();

private:
    const Archetype& readArchetype(const EntitySignature& signature) const;
    Archetype& editArchetype(const EntitySignature& signature);
    Archetype& editOrCreateArchetype(const EntitySignature& signature);
    Archetype& prepareArchetypeOnAddComponent(Entity entity, TypeId componentId);

    WorldHandle m_handle;
    std::unordered_map<SystemCallbackHandle, SystemCallback> m_systemCallbacks{};
    Entity::ValueType m_nextEntityValue{};
    std::unordered_map<Entity, EntitySignature> m_entities{};
    std::unordered_map<EntitySignature, Archetype> m_archetypes;
    RenderManager* m_renderManager{};
};

//------------------------------------------------------------------------------------------------------------------------
// World - Implementation
//------------------------------------------------------------------------------------------------------------------------

template <ValidComponentData T, typename... Args>
T& World::addComponent(Entity entity, Args&&... args)
{
    assertThread();
    Archetype& archetype = prepareArchetypeOnAddComponent(entity, Component<T>::typeId());
    T& addedComponent = archetype.addComponent<T>(entity, T{std::forward<Args>(args)...});
    log(std::format("Added component {} to entity {}", getComponentName<T>(), entity));

    for (SystemCallback& callback : m_systemCallbacks | std::views::values)
    {
        callback.onComponentAdded(entity, Component<T>::typeId());
    }
    return addedComponent;
}

template <ValidComponentData T>
T& World::addComponent(Entity entity, T&& args)
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
    report(std::format("{} was requested for an entity that doesn't exist", getTypeName<T>()));
    return false;
}

template <ValidComponentData T>
[[nodiscard]] const T& World::readComponent(Entity entity) const
{
    if (auto it = m_entities.find(entity); it != m_entities.end())
    {
        return readArchetype(it->second).readComponent<T>(entity);
    }
    fatalError(std::format("Couldn't find component: {}", getComponentName<T>()));
    static const T invalid{};
    return invalid;
}

template <ValidComponentData First, ValidComponentData ... Rest>
std::generator<std::tuple<Entity, const First&, const Rest&...>> World::view() const
{
    for (auto& archetype : m_archetypes | std::views::values)
    {
        if (archetype.matches<First, Rest...>())
        {
            for (auto&& entityComponents : archetype.view<First, Rest...>())
            {
                co_yield entityComponents;
            }
        }
    }
}

template <ValidComponentData First, ValidComponentData ... Rest>
std::generator<std::tuple<Entity, First&, Rest&...>> World::view()
{
    for (auto& archetype : m_archetypes | std::views::values)
    {
        if (archetype.matches<First, Rest...>())
        {
            for (auto&& entityComponents : archetype.view<First, Rest...>())
            {
                co_yield entityComponents;
            }
        }
    }
}
