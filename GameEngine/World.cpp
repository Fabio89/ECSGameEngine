module Engine:World;
import :Config;
import :AssetManager;

template <typename T>
void loadAssets(const JsonObject& json, const char* assetName)
{
    if (auto assets = json.FindMember(assetName); assets != json.MemberEnd())
    {
        for (const JsonObject& element : assets->value.GetArray())
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
    
    if (auto entities = cfg.FindMember("entities"); entities != cfg.MemberEnd())
    {
        for (const JsonObject& entityJson : entities->value.GetArray())
        {
            Entity entity = createEntity();

            if (auto components = entityJson.FindMember("components"); components != entityJson.MemberEnd() && components->value.IsObject())
            {
                for (auto it = components->value.MemberBegin(); it != components->value.MemberEnd(); ++it)
                {
                    const std::string& typeName = it->name.GetString();
                    const JsonObject& componentData = it->value.GetObject();
                    if (const ComponentTypeBase* componentType = ComponentRegistry::get(typeName))
                    {
                        componentType->createInstance(*this, entity, componentData);
                    }
                }
            }
        }
    }
}

World::World(const ApplicationSettings& settings, IRenderManager* renderManager)
    : m_jobSystem{settings.numThreads},
      m_renderManager{*renderManager}
{
}

Entity World::createEntity()
{
    Entity entity = m_nextEntity++;
    m_entities.try_emplace(entity);
    return entity;
}

void World::addDebugWidget(std::unique_ptr<DebugWidget> widget)
{
    m_renderManager.get().addDebugWidget(std::move(widget));
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
