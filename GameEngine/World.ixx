module;

export module Engine.World;

import Engine.Config;
import Engine.Core;
import Engine.Job;
import Engine.Render;
import std;
import std.compat;

import Engine.Render.Core;

static constexpr int maxComponentsPerEntity = 64;

struct EntitySignature
{
    friend bool operator==(const EntitySignature& a, const EntitySignature& b) { return a.bitset == b.bitset; }
    std::bitset<maxComponentsPerEntity> bitset;
};

template <>
struct std::hash<EntitySignature>
{
    size_t operator()(const EntitySignature& a) const noexcept
    {
        return hash<bitset<maxComponentsPerEntity>>()(a.bitset);
    }
};

class RenderObjectManager;

export class World
{
public:
    World(const ApplicationSettings&, ApplicationState& globalState);

    Entity createEntity();

    template <typename T, typename... Args>
    void addComponent(Entity entity, Args&&... args);

    template <typename T>
    const T& readComponent(Entity entity) const;

    template <typename T>
    T& editComponent(Entity entity);

    void removeEntity(Entity entity);

    void addSystem(std::unique_ptr<System> system);

    void updateSystems(float deltaTime);

    ArchetypeChangedObserverHandle observeOnComponentAdded(ArchetypeChangedCallback observer);
    void unobserveOnComponentAdded(ArchetypeChangedObserverHandle observerHandle);

    void createObjectsFromConfig();

private:
    const Archetype& readArchetype(const EntitySignature& signature) const;
    Archetype& editArchetype(const EntitySignature& signature);
    Archetype& editOrCreateArchetype(const EntitySignature& signature);

    std::vector<std::unique_ptr<AssetBase>> m_loadedAssets;
    std::unordered_map<ArchetypeChangedObserverHandle, ArchetypeChangedCallback> m_archetypeChangeObservers;
    JobSystem m_jobSystem;
    Entity m_nextEntity = 0;
    std::unordered_map<Entity, EntitySignature> m_entities;
    std::unordered_map<EntitySignature, Archetype> m_archetypes;
    std::vector<std::unique_ptr<System>> m_systems;
    std::reference_wrapper<ApplicationState> m_applicationState;
};

template <typename T, typename... Args>
void World::addComponent(Entity entity, Args&&... args)
{
    EntitySignature& signature = m_entities[entity];
    Archetype& oldArchetype = editArchetype(signature);
    oldArchetype.removeEntity(entity);
    if (oldArchetype.isEmpty())
    {
        m_archetypes.erase(signature);
    }

    signature.bitset.set(Component<T>::typeId.hash_code() % maxComponentsPerEntity);
    editOrCreateArchetype(signature).addComponent<T>(entity, T(std::forward<Args>(args)...));

    for (auto& callback : m_archetypeChangeObservers | std::views::values)
    {
        callback(entity, Component<T>::typeId);
    }
}

template <typename T>
const T& World::readComponent(Entity entity) const
{
    if (auto it = m_entities.find(entity); it != m_entities.end())
    {
        return readArchetype(it->second).readComponent<T>(entity);
    }
    static const T invalid{};
    return invalid;
}

template <typename T>
T& World::editComponent(Entity entity)
{
    return const_cast<T&>(readComponent<T>(entity));
}
