export module World;
import Archetype;
import Core;
import DebugUI.IDebugWidget;
import Guid;
import Render.RenderManager;
import Serialization;

static constexpr int maxComponentsPerEntity = 64;

struct EntitySignature
{
    friend bool operator==(const EntitySignature& a, const EntitySignature& b) { return a.bitset == b.bitset; }
    std::bitset<maxComponentsPerEntity> bitset{};
};

template <>
struct std::hash<EntitySignature>
{
    size_t operator()(const EntitySignature& a) const noexcept { return hash<bitset<maxComponentsPerEntity>>()(a.bitset); }
};

export struct WorldCreateInfo
{
    RenderManager* renderManager{};
};

//------------------------------------------------------------------------------------------------------------------------
// World
//------------------------------------------------------------------------------------------------------------------------
export class World
{
public:
    explicit World(const WorldCreateInfo&);

    Entity createEntity();
    void removeEntity(Entity entity);

    void loadScene(std::string_view path);
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
    bool hasComponent(Entity entity, ComponentTypeId componentTypeId) const;

    template <ValidComponentData T>
    const T& readComponent(Entity entity) const;

    const ComponentBase& readComponent(Entity entity, ComponentTypeId componentType) const;

    Archetype::ComponentRange getComponentTypesInEntity(Entity entity) const;

    template <ValidComponentData T>
    T& editComponent(Entity entity) { return const_cast<T&>(readComponent<T>(entity)); }

    template <typename T>
    void addDebugWidget() { addDebugWidget(std::make_unique<T>(*this)); }

    ArchetypeChangedObserverHandle observeOnComponentAdded(ArchetypeChangedCallback observer);
    void unobserveOnComponentAdded(ArchetypeChangedObserverHandle observerHandle);

    auto getEntitiesRange() const { return m_entities | std::views::keys; }

    template <ValidComponentData First, ValidComponentData ... Rest>
    std::generator<std::tuple<Entity, const First&, const Rest&...>> view() const;

    template <ValidComponentData First, ValidComponentData ... Rest>
    std::generator<std::tuple<Entity, First&, Rest&...>> view();

    const RenderManager& getRenderManager() const { return m_renderManager.get(); }
    RenderManager& getRenderManager() { return m_renderManager.get(); }

    void printArchetypeStatus();

private:
    void addDebugWidget(std::unique_ptr<IDebugWidget> widget);
    const Archetype& readArchetype(const EntitySignature& signature) const;
    Archetype& editArchetype(const EntitySignature& signature);
    Archetype& editOrCreateArchetype(const EntitySignature& signature);

    std::unordered_map<ArchetypeChangedObserverHandle, ArchetypeChangedCallback> m_archetypeChangeObservers;
    Entity m_nextEntity = 0;
    std::unordered_map<Entity, EntitySignature> m_entities;
    std::unordered_map<EntitySignature, Archetype> m_archetypes;
    std::reference_wrapper<RenderManager> m_renderManager;
};

//------------------------------------------------------------------------------------------------------------------------
// World - Implementation
//------------------------------------------------------------------------------------------------------------------------
template <ValidComponentData T, typename... Args>
T& World::addComponent(Entity entity, Args&&... args)
{
    EntitySignature& signature = m_entities[entity];
    const EntitySignature oldSignature = signature;

    Archetype& oldArchetype = m_archetypes[oldSignature];

    signature.bitset.set(std::hash<ComponentTypeId>{}(Component<T>::typeId()) % maxComponentsPerEntity);

    const bool usingExistingArchetype = m_archetypes.contains(signature);

    Archetype& newArchetype = m_archetypes[signature];
    if (usingExistingArchetype)
    {
        newArchetype.steal(oldArchetype, entity);
    }
    else
    {
        newArchetype = oldArchetype.cloneForEntity(entity);
        oldArchetype.removeEntity(entity);
    }

    if (oldArchetype.isEmpty())
    {
        m_archetypes.erase(oldSignature);
    }

    T& addedComponent = newArchetype.addComponent<T>(entity, T{std::forward<Args>(args)...});
    log(std::format("Added component {} to entity {}", getComponentName<T>(), entity));

    for (auto& callback : m_archetypeChangeObservers | std::views::values)
    {
        callback(entity, Component<T>::typeId());
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
    report("A component was requested for an entity that doesn't exist");
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
