module World;
import Assets.Mesh;
import Assets.Texture;
import AssetManager;
import ComponentRegistry;
import Components.Camera;
import Components.Name;
import Components.Tags;
import Thread;
import World.Events;

World::World(const WorldCreateInfo& info)
    : m_handle{info.handle} {}

Entity World::createEntity()
{
    assertThread();
    const Entity entity{m_nextEntityValue++};
    m_entities.try_emplace(entity);
    m_eventBus.publish(WorldEvents::EntityCreated{.world = m_handle, .entity = entity});
    return entity;
}

void World::nextFrame()
{
    m_dirtyTracker.nextFrame();
}

void World::printArchetypeStatus()
{
    for (const Archetype& archetype : m_archetypes | std::views::values)
    {
        std::string archetypeStr = "Archetype [";
        std::string separator;

        for (auto componentType : archetype.getComponentTypes())
        {
            archetypeStr += separator;
            archetypeStr += ComponentRegistry::get(componentType)->getName();

            separator = " | ";
        }

        archetypeStr += "]\nEntities: ";

        for (auto&& [entity] : archetype.view())
        {
            archetypeStr += NameUtils::getName(*this, entity);
            archetypeStr += " ";
        }
        archetypeStr += "\n";
        log(archetypeStr);
    }
}

const Archetype& World::readArchetype(const EntitySignature& signature) const
{
    if (auto it = m_archetypes.find(signature); it != m_archetypes.end())
    {
        return it->second;
    }

    static const Archetype emptyArchetype{};
    return emptyArchetype;
    //throw std::runtime_error{"Couldn't find requested archetype!\n\tRequested: " + signature.bitset.to_string() + "\n"};
}

Archetype& World::editArchetype(const EntitySignature& signature)
{
    return const_cast<Archetype&>(std::as_const(*this).readArchetype(signature));
}

Archetype& World::editOrCreateArchetype(const EntitySignature& signature)
{
    return m_archetypes[signature];
}

UInt64 getComponentIndex(TypeId componentId)
{
    static std::unordered_map<TypeId, UInt64> componentIndexMap;
    static UInt64 lastComponentIndex = 0u;
    
    auto [it, added] = componentIndexMap.try_emplace(componentId, lastComponentIndex);
    if (added)
        ++lastComponentIndex;
    check(!added || it->second < maxComponentsPerEntity, std::format("Can't have more than {} components!", maxComponentsPerEntity));
    return it->second;
}

Archetype& World::prepareArchetypeOnAddComponent(Entity entity, TypeId componentId)
{
    EntitySignature& signature = m_entities[entity];
    const EntitySignature oldSignature = signature;

    Archetype& oldArchetype = m_archetypes[oldSignature];

    signature.bitset.set(getComponentIndex(componentId));

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

    return newArchetype;
}

void World::removeEntity(Entity entity)
{
    assertThread();
    auto it = m_entities.find(entity);
    if (it != m_entities.end())
    {
        Archetype& archetype = editArchetype(it->second);
        archetype.removeEntity(entity);
        if (archetype.isEmpty())
            m_archetypes.erase(it->second);

        m_entities.erase(it);
        m_dirtyTracker.remove(entity);
        m_eventBus.publish(WorldEvents::EntityDestroyed{.world = m_handle, .entity = entity});
    }
}

bool World::isValid(Entity entity) const
{
    return entity.isValid() && m_entities.contains(entity);
}

void World::removeAllEntities()
{
    assertThread();
    m_entities.clear();
    m_archetypes.clear();
    m_dirtyTracker = {};
    m_eventBus.publish(WorldEvents::WorldCleared{.world = m_handle});
}

bool World::hasComponent(Entity entity, TypeId componentTypeId) const
{
    if (auto it = m_entities.find(entity); it != m_entities.end())
    {
        return std::ranges::contains(readArchetype(it->second).getComponentTypes(), componentTypeId);
    }
    report(std::format("{} was requested for entity {} which doesn't exist", ComponentRegistry::get(componentTypeId)->getName(), entity));
    return false;
}

[[nodiscard]]
const ComponentBase& World::getComponent(Entity entity, TypeId componentType) const
{
    if (auto it = m_entities.find(entity); it != m_entities.end())
    {
        return readArchetype(it->second).readComponent(entity, componentType);
    }
    fatalError(std::format("Couldn't find component with id: {}", componentType));
    static constexpr ComponentBase invalid{};
    return invalid;
}

[[nodiscard]]
Archetype::ComponentRange World::getComponentTypesInEntity(Entity entity) const
{
    if (auto it = m_entities.find(entity); it != m_entities.end())
    {
        return readArchetype(it->second).getComponentTypes();
    }
    fatalError(std::format("Couldn't find components for entity {}", entity));
    static const Archetype::ComponentArrayMap emptyMap;
    return emptyMap | std::views::keys;
}
