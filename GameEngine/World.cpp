module Engine.World;

import Engine.ApplicationState;
import Engine.AssetManager;
import Engine.ComponentRegistry;
import Engine.DebugWidget;
import Engine.Guid;
import Engine.Components;
import Engine.Config;
import Engine.ImGui;
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
                if (const ComponentTypeBase* componentType = ComponentRegistry::get(typeName))
                {
                    componentType->createInstance(*this, entity, componentData);
                }
            }
        }
    }
}

World::World(const ApplicationSettings& settings, ApplicationState& globalState)
    : m_jobSystem{settings.numThreads},
      m_applicationState{globalState}
{
    globalState.world = this;
}

Entity World::createEntity()
{
    Entity entity = m_nextEntity++;
    m_entities.try_emplace(entity);
    return entity;
}

void World::addDebugWidget(std::unique_ptr<DebugWidget> widget)
{
    m_applicationState.get().debugUI->addWidget(std::move(widget));
}

const Archetype& World::readArchetype(const EntitySignature& signature) const
{
    if (auto it = m_archetypes.find(signature); it != m_archetypes.end())
    {
        return it->second;
    }
    
    throw std::runtime_error{"Couldn't find requested archetype!\n\tRequested: " + signature.bitset.to_string() + "\n"};
}

Archetype& World::editArchetype(const EntitySignature& signature)
{
    return const_cast<Archetype&>(std::as_const(*this).readArchetype(signature));
}

Archetype& World::editOrCreateArchetype(const EntitySignature& signature)
{
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
