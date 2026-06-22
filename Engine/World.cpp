module World;
import AssetManager;
import ComponentRegistry;
import Component.Name;
import Component.Tags;
import Render.Model;

template <typename T>
std::vector<const T*> loadAssets(const JsonObject& json, const char* assetName)
{
    std::vector<const T*> loadedAssets;

    if (json.IsObject())
    {
        if (const auto assets = json.FindMember(assetName); assets != json.MemberEnd())
        {
            loadedAssets.reserve(assets->value.Size());
            for (const JsonObject& element : assets->value.GetArray())
            {
                if (const T* asset = AssetManager::loadAsset<T>(element))
                    loadedAssets.push_back(asset);
            }
        }
    }

    return loadedAssets;
}

World::World(const WorldCreateInfo& info)
    : m_renderManager{*info.renderManager}
{
}

Entity World::createEntity()
{
    const Entity entity = m_nextEntity++;
    m_entities.try_emplace(entity);
    return entity;
}

void World::printArchetypeStatus()
{
    for (const Archetype& archetype : m_archetypes | std::views::values)
    {
        std::string archetypeStr = "Archetype [";

        auto componentTypes = archetype.getComponentTypes();
        auto lastComponentTypeIt = std::prev(componentTypes.end());
        for (auto it = componentTypes.begin(); it != componentTypes.end(); ++it)
        {
            archetypeStr += ComponentRegistry::get(*it)->getName();
            if (it != lastComponentTypeIt)
            {
                archetypeStr += " | ";
            }
        }
        archetypeStr += "]\nEntities: ";

        auto entities = archetype.view();
        for (auto&& [entity] : archetype.view())
        {
            std::string entityName = hasComponent<NameComponent>(entity) ? std::format("'{}'", readComponent<NameComponent>(entity).name) : std::format("'{}'", entity);
            archetypeStr += entityName + " ";
        }
        archetypeStr += "\n";
        log(archetypeStr);
    }
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

UInt64 getComponentIndex(ComponentTypeId componentId)
{
    static std::unordered_map<ComponentTypeId, UInt64> componentIndexMap;
    static UInt64 lastComponentIndex = 0u;
    
    auto [it, added] = componentIndexMap.try_emplace(componentId, lastComponentIndex++);
    check(!added || it->second < maxComponentsPerEntity, std::format("Can't have more than {} components!", maxComponentsPerEntity));
    return it->second;
}

Archetype& World::prepareArchetypeOnAddComponent(Entity entity, ComponentTypeId componentId)
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
    auto it = m_entities.find(entity);
    if (it != m_entities.end())
    {
        Archetype& archetype = editArchetype(it->second);
        archetype.removeEntity(entity);
        if (archetype.isEmpty())
        {
            m_archetypes.erase(it->second);
        }
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
        if (hasComponent<TagsComponent>(entity) && std::ranges::contains(readComponent<TagsComponent>(entity).tags, Tag::notEditable))
            continue;
        
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

    for (const MeshAsset* mesh : loadAssets<MeshAsset>(json, "meshes"))
    {
        m_renderManager.get().addCommand(RenderCommands::AddMesh{mesh->getGuid(), mesh->getData()});
    }
    for (const TextureAsset* texture : loadAssets<TextureAsset>(json, "textures"))
    {
        m_renderManager.get().addCommand(RenderCommands::AddTexture{texture->getGuid(), texture->getData()});
    }
    
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

bool World::hasComponent(Entity entity, ComponentTypeId componentTypeId) const
{
    if (auto it = m_entities.find(entity); it != m_entities.end())
    {
        return std::ranges::contains(readArchetype(it->second).getComponentTypes(), componentTypeId);
    }
    report("A component was requested for an entity that doesn't exist");
    return false;
}

[[nodiscard]]
const ComponentBase& World::readComponent(Entity entity, ComponentTypeId componentType) const
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
