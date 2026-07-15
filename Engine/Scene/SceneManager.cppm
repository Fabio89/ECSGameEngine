export module SceneManager;
import Core;
import EventBus;
import Engine.WorldManager;
import WorldHandle;

export class SceneManager
{
public:
    explicit SceneManager(WorldManager& worldManager);

    void loadScene(WorldHandle worldHandle, const std::filesystem::path& scenePath);
    void unloadScene(WorldHandle worldHandle);

private:
    WorldManager& m_worlds;
    EventBus m_eventBus;
};
