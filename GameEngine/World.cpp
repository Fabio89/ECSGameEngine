module Engine.World;

import Engine.AssetManager;
import Engine.ComponentRegistry;
import Engine.Guid;
import Engine.Components;
import Engine.Config;
import Engine.Render.Application;
import Engine.Render.Core;
import std;
import <glm/glm.hpp>;

template <typename T>
void loadAssets(const Json& json, const char* assetName)
{
    if (auto assets = json.find(assetName); assets != json.end())
    {
        for (const Json& element : *assets)
        {
            AssetManager::loadAsset<T>(element);
        }
    }
}

void System::update(float deltaTime)
{
    for (auto& func : m_updateFunctions)
    {
        func(deltaTime);
    }
}

void System::addUpdateFunction(std::function<void(float)> func)
{
    m_updateFunctions.push_back(std::move(func));
}

void World::createObjectsFromConfig()
{
    EngineComponents::init();

    const auto& cfg = Config::getEngineConfig();

    loadAssets<MeshAsset>(cfg, "meshes");
    loadAssets<TextureAsset>(cfg, "textures");

    if (auto entities = cfg.find("entities"); entities != cfg.end())
    {
        for (const auto& entityJson : *entities)
        {
            Entity entity = createEntity();
            for (auto it = entityJson["components"].begin(); it != entityJson["components"].end(); ++it)
            {
                const std::string& typeName = it.key();
                const Json& componentData = it.value();
                if (auto createFunc = ComponentRegistry::getComponentCreateFunc(typeName))
                {
                    createFunc(*this, entity, componentData);
                }
            }
        }
    }
}

World::World(const ApplicationSettings& settings, ApplicationState& globalState)
    : m_jobSystem{settings.numThreads},
      m_applicationState{globalState}
{
}

Entity World::createEntity()
{
    Entity entity = m_nextEntity++;
    m_entities.try_emplace(entity);
    return entity;
}

const Archetype& World::readArchetype(const EntitySignature& signature) const
{
    if (auto it = m_archetypes.find(signature); it != m_archetypes.end())
    {
        return it->second;
    }
    static const Archetype invalid;
    return invalid;
}

Archetype& World::editArchetype(const EntitySignature& signature)
{
    return const_cast<Archetype&>(std::as_const(*this).readArchetype(signature));
}

Archetype& World::editOrCreateArchetype(const EntitySignature& signature)
{
    if (!m_archetypes.contains(signature))
    {
        m_archetypes[signature] = Archetype();
    }
    return m_archetypes[signature];
}

void World::removeEntity(Entity entity)
{
    auto it = m_entities.find(entity);
    if (it != m_entities.end())
    {
        editArchetype(it->second).removeEntity(entity);
        m_entities.erase(it);
    }
}

void World::addSystem(std::unique_ptr<System> system)
{
    System* systemPtr = m_systems.emplace_back(std::move(system)).get();
    observeOnComponentAdded([&](Entity entity, ComponentTypeId componentId)
    {
        systemPtr->onComponentAdded(*this, entity, componentId);
    });
}

void World::updateSystems(float deltaTime)
{
    for (auto& system : m_systems)
    {
        system->update(deltaTime);
    }
}

ArchetypeChangedObserverHandle World::observeOnComponentAdded(ArchetypeChangedCallback observer)
{
    ArchetypeChangedObserverHandle handle = generateArchetypeObserverHandle();
    m_archetypeChangeObservers.insert_or_assign(handle, std::move(observer));
    return handle;
}

void World::unobserveOnComponentAdded(ArchetypeChangedObserverHandle observerHandle)
{
    m_archetypeChangeObservers.erase(observerHandle);
}
