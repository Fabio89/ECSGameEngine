module Engine:World;
import :AssetManager;
import :Components;
import :System;
import Serialization;

template <typename T>
void loadAssets(const JsonObject& json, const char* assetName)
{
    if (!json.IsObject())
        return;

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

World::World(const WorldCreateInfo& info)
    : m_renderManager{*info.renderManager}
{
}

Entity World::createEntity()
{
    Entity entity = m_nextEntity++;
    m_entities.try_emplace(entity);
    return entity;
}

void World::addDebugWidget(std::unique_ptr<IDebugWidget> widget)
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

void World::loadScene(std::string_view path)
{
    std::cout << "Loading scene: " << path << '\n';
    const JsonObject& doc = Json::fromFile(path);
    deserializeScene(doc);
    std::cout << "Scene loading complete\n";
}

void World::unloadScene()
{
    m_renderManager.get().clear();
    m_entities.clear();
    m_archetypes.clear();
    for (auto& system : m_systems)
        system->clear();
}

[[nodiscard]]
JsonObject World::serializeScene(Json::MemoryPoolAllocator<>& allocator) const
{
    JsonObject jsonScene{Json::kObjectType};
    JsonObject jsonEntityArray{Json::kArrayType};

    check(std::in_range<Json::SizeType>(m_entities.size()), "Truncating value of m_entities.size()!");
    jsonEntityArray.Reserve(static_cast<Json::SizeType>(m_entities.size()), allocator);

    for (auto entity : m_entities | std::views::keys)
    {
        JsonObject jsonEntity{Json::kObjectType};
        JsonObject jsonComponentDict{Json::kObjectType};

        auto components = getComponentTypesInEntity(entity);
        check(std::in_range<Json::SizeType>(components.size()), "Truncating value of components.size()!");
        jsonComponentDict.MemberReserve(static_cast<Json::SizeType>(components.size()), allocator);

        for (auto componentType : components)
        {
            const ComponentBase& component = readComponent(entity, componentType);
            const ComponentTypeBase& componentTypeInfo = *ComponentRegistry::get(componentType);
            JsonObject jsonComponent = componentTypeInfo.serialize(component, allocator);
            jsonComponentDict.AddMember(Json::GenericStringRef{componentTypeInfo.getName().data()}, jsonComponent, allocator);
        }
        jsonEntity.AddMember("id", entity, allocator);
        jsonEntity.AddMember("components", jsonComponentDict, allocator);
        jsonEntityArray.PushBack(jsonEntity, allocator);
    }

    jsonScene.AddMember("entities", jsonEntityArray, allocator);
    return jsonScene;
}

void World::deserializeScene(const JsonObject& json)
{
    unloadScene();

    if (!json.IsObject())
        return;

    loadAssets<MeshAsset>(json, "meshes");
    loadAssets<TextureAsset>(json, "textures");

    if (auto entities = json.FindMember("entities"); entities != json.MemberEnd())
    {
        for (const JsonObject& entityJson : entities->value.GetArray())
        {
            const Entity entity = createEntity();

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

void World::patchEntity(Entity entity, const JsonObject& json)
{
    if (!json.IsObject())
    {
        log("Failed to patch entity!");
        return;
    }
    
    for (auto it = json.MemberBegin(); it != json.MemberEnd(); ++it)
    {
        const std::string& typeName = it->name.GetString();
        const JsonObject& componentData = it->value.GetObject();

        if (const ComponentTypeBase* componentType = ComponentRegistry::get(typeName))
        {
            componentType->deserialize(*this, entity, componentData);
            log(std::format("Patched component: {}", componentType->getName()));
        }
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
