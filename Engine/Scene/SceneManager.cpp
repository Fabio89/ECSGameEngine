module SceneManager;
import Scene.Events;
import ComponentRegistry;
import Serialization.Json;
import World;

SceneManager::SceneManager(WorldManager& worldManager) : m_worlds{worldManager} {}

void SceneManager::loadScene(WorldHandle worldHandle, const std::filesystem::path& scenePath)
{
    World& world = m_worlds.get(worldHandle);
    world.removeAllEntities();

    const JsonObject& json = Json::fromFile(scenePath);

    if (!json.IsObject())
        return;

    if (auto entities = json.FindMember("entities"); entities != json.MemberEnd())
    {
        for (const JsonObject& entityJson : entities->value.GetArray())
        {
            const Entity entity = world.createEntity();

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

    m_eventBus.publish(SceneEvents::Loaded{.world = worldHandle});
}

void SceneManager::unloadScene(WorldHandle worldHandle)
{
    World& world = m_worlds.get(worldHandle);
    world.removeAllEntities();
}
