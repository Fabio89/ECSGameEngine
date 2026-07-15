module Scene;
import Components.Tags;
import ComponentRegistry;

Scene::Scene(World& world, const std::filesystem::path& path)
{
    deserialize(world, path);
}

void Scene::deserialize(World& world, const std::filesystem::path& path)
{
    clear(world);

    const JsonObject& json = Json::fromFile(path);

    if (!json.IsObject())
        return;

    if (auto entities = json.FindMember("entities"); entities != json.MemberEnd())
    {
        for (const JsonObject& entityJson : entities->value.GetArray())
        {
            const Entity entity = m_entities.emplace_back(world.createEntity());

            if (auto components = entityJson.FindMember("components"); components != entityJson.MemberEnd() && components->value.IsObject())
            {
                for (auto it = components->value.MemberBegin(); it != components->value.MemberEnd(); ++it)
                {
                    const std::string& typeName = it->name.GetString();
                    const JsonObject& componentData = it->value.GetObject();
                    if (const ComponentTypeBase* componentType = ComponentRegistry::get(typeName))
                    {
                        componentType->createInstance(world, entity, componentData);
                    }
                }
            }
        }
    }
}

void Scene::serialize(const World& world, const std::filesystem::path& path)
{
    JsonObject jsonScene{Json::kObjectType};
    JsonObject jsonEntityArray{Json::kArrayType};

    check(std::in_range<Json::SizeType>(m_entities.size()), "Truncating value of m_entities.size()!");

    JsonDocument doc;
    auto& allocator = doc.GetAllocator();

    jsonEntityArray.Reserve(static_cast<Json::SizeType>(m_entities.size()), allocator);

    for (Entity entity : m_entities)
    {
        if (world.hasComponent<TagsComponent>(entity) && std::ranges::contains(world.readComponent<TagsComponent>(entity).tags, Tag::editorOnly))
            continue;

        JsonObject jsonEntity{Json::kObjectType};
        JsonObject jsonComponentDict{Json::kObjectType};

        auto components = world.getComponentTypesInEntity(entity);
        check(std::in_range<Json::SizeType>(components.size()), "Truncating value of components.size()!");
        jsonComponentDict.MemberReserve(static_cast<Json::SizeType>(components.size()), allocator);

        for (auto componentType : components)
        {
            const ComponentBase& component = world.readComponent(entity, componentType);
            const ComponentTypeBase& componentTypeInfo = *ComponentRegistry::get(componentType);
            JsonObject jsonComponent = componentTypeInfo.serialize(component, allocator);
            jsonComponentDict.AddMember(Json::GenericStringRef{componentTypeInfo.getName().data()}, jsonComponent, allocator);
        }
        jsonEntity.AddMember("id", entity.value, allocator);
        jsonEntity.AddMember("components", jsonComponentDict, allocator);
        jsonEntityArray.PushBack(jsonEntity, allocator);
    }

    jsonScene.AddMember("entities", jsonEntityArray, allocator);
    doc.AddMember("scene", jsonScene, allocator);
    Json::toFile(doc, path);
}

void Scene::clear(World& world)
{
    world.removeAllEntities();
    m_entities.clear();
}
