export module World;
export import Ecs;
import DebugUI.IDebugWidget;
import Render.IRenderManager;
import Serialization;
import std;

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
    IRenderManager* renderManager{};
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
    
    template <ValidComponent T, typename... Args>
    void addComponent(Entity entity, Args&&... args);

    template <ValidComponent T>
    const T& readComponent(Entity entity) const;

    const ComponentBase& readComponent(Entity entity, ComponentTypeId componentType) const;

    Archetype::ComponentRange getComponentTypesInEntity(Entity entity) const;

    template <ValidComponent T>
    T& editComponent(Entity entity) { return const_cast<T&>(readComponent<T>(entity)); }
    
    template <typename T>
    void addDebugWidget() { addDebugWidget(std::make_unique<T>(*this)); }

    ArchetypeChangedObserverHandle observeOnComponentAdded(ArchetypeChangedCallback observer);
    void unobserveOnComponentAdded(ArchetypeChangedObserverHandle observerHandle);

    auto getEntitiesRange() const { return m_entities | std::views::keys; }

    template <ValidComponent ... Components>
    std::generator<std::tuple<Entity, const Components&...>> view() const;

    template <ValidComponent ... Components>
    std::generator<std::tuple<Entity, Components&...>> view();
    
    const IRenderManager& getRenderManager() const { return m_renderManager.get(); }
    IRenderManager& getRenderManager() { return m_renderManager.get(); }

private:
    void addDebugWidget(std::unique_ptr<IDebugWidget> widget);
    const Archetype& readArchetype(const EntitySignature& signature) const;
    Archetype& editArchetype(const EntitySignature& signature);
    Archetype& editOrCreateArchetype(const EntitySignature& signature);
    
    std::unordered_map<ArchetypeChangedObserverHandle, ArchetypeChangedCallback> m_archetypeChangeObservers;
    Entity m_nextEntity = 0;
    std::unordered_map<Entity, EntitySignature> m_entities;
    std::unordered_map<EntitySignature, Archetype> m_archetypes;
    std::reference_wrapper<IRenderManager> m_renderManager;
};

//------------------------------------------------------------------------------------------------------------------------
// World - Implementation
//------------------------------------------------------------------------------------------------------------------------

template <ValidComponent T, typename ... Args>
void World::addComponent(Entity entity, Args&&... args)
{
    EntitySignature& signature = m_entities[entity];
    const EntitySignature oldSignature = signature;

    Archetype& oldArchetype = editOrCreateArchetype(signature);

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
    newArchetype.addComponent<T>(entity, T{std::forward<Args>(args)...});

    if (oldArchetype.isEmpty())
    {
        m_archetypes.erase(oldSignature);
    }

    for (auto& callback : m_archetypeChangeObservers | std::views::values)
    {
        callback(entity, Component<T>::typeId());
    }
}

template <ValidComponent T>
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

template <ValidComponent ... Components>
std::generator<std::tuple<Entity, const Components&...>> World::view() const
{
    for (auto& archetype : m_archetypes | std::views::values)
    {
        if (archetype.matches<Components...>())
        {
            for (auto&& entityComponents : archetype.view<Components...>())
            {
                co_yield entityComponents;
            }
        }
    }
}

template <ValidComponent ... Components>
std::generator<std::tuple<Entity, Components&...>> World::view()
{
    for (auto& archetype : m_archetypes | std::views::values)
    {
        if (archetype.matches<Components...>())
        {
            for (auto&& entityComponents : archetype.view<Components...>())
            {
                co_yield entityComponents;
            }
        }
    }   
}
