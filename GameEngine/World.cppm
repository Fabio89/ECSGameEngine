export module Engine:World;
import :Core;
import :Job;
import :Render.IRenderManager;
import :System;

export class World;

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

export class World
{
public:
    World(const WorldCreateInfo&);

    Entity createEntity();
    void removeEntity(Entity entity);

    void loadScene(std::string_view path);
    void loadScene(const JsonObject& json);
    JsonObject serializeScene(Json::MemoryPoolAllocator<>& allocator) const;
    void unloadScene();

    template <ValidComponent T, typename... Args>
    void addComponent(Entity entity, Args&&... args)
    {
        EntitySignature& signature = m_entities[entity];
        const EntitySignature oldSignature = signature;

        Archetype& oldArchetype = editOrCreateArchetype(signature);

        signature.bitset.set(Component<T>::typeId.hash_code() % maxComponentsPerEntity);

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
        newArchetype.addComponent<T>(entity, T(std::forward<Args>(args)...));

        if (oldArchetype.isEmpty())
        {
            m_archetypes.erase(oldSignature);
        }

        for (auto& callback : m_archetypeChangeObservers | std::views::values)
        {
            callback(entity, Component<T>::typeId);
        }
    }

    template <ValidComponent T>
    [[nodiscard]] const T& readComponent(Entity entity) const
    {
        return readComponent<T>(entity, Component<T>::typeId);
    }

    template <ValidComponent T>
    [[nodiscard]] const T& readComponent(Entity entity, ComponentTypeId componentType) const
    {
        if (auto it = m_entities.find(entity); it != m_entities.end())
        {
            return readArchetype(it->second).readComponent<T>(entity, componentType);
        }
        fatalError(std::format("Couldn't find component: {}", getComponentName<T>()));
        static const T invalid{};
        return invalid;
    }

    [[nodiscard]] const ComponentBase& readComponent(Entity entity, ComponentTypeId componentType) const
    {
        if (auto it = m_entities.find(entity); it != m_entities.end())
        {
            return readArchetype(it->second).readComponent(entity, componentType);
        }
        fatalError(std::format("Couldn't find component: {}", componentType.name()));
        static constexpr ComponentBase invalid{};
        return invalid;
    }

    [[nodiscard]] Archetype::ComponentRange getComponentTypesInEntity(Entity entity) const
    {
        if (auto it = m_entities.find(entity); it != m_entities.end())
        {
            return readArchetype(it->second).getComponentTypes();
        }
        fatalError(std::format("Couldn't find components for entity {}", entity));
        static const Archetype::ComponentArrayMap emptyMap;
        return emptyMap | std::views::keys;
    }

    template <ValidComponent T>
    T& editComponent(Entity entity) { return const_cast<T&>(readComponent<T>(entity)); }

    void addSystem(std::unique_ptr<System> system);

    template <typename T>
    void addSystem()
    {
        System* system = m_systems.emplace_back(std::make_unique<T>()).get();
        observeOnComponentAdded([system, this](Entity entity, ComponentTypeId componentId)
        {
            system->onComponentAdded(*this, entity, componentId);
        });
    }

    void updateSystems(float deltaTime);

    template <typename T>
    void addDebugWidget() { addDebugWidget(std::make_unique<T>(*this)); }

    ArchetypeChangedObserverHandle observeOnComponentAdded(ArchetypeChangedCallback observer);
    void unobserveOnComponentAdded(ArchetypeChangedObserverHandle observerHandle);

    auto getEntitiesRange() const { return m_entities | std::views::keys; }

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
    std::vector<std::unique_ptr<System>> m_systems;
    std::reference_wrapper<IRenderManager> m_renderManager;
};
