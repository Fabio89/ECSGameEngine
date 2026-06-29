export module World.Events;
import World;

export namespace Engine
{
    struct SceneLoadedEvent
    {
        std::reference_wrapper<World> world;
    };

    struct EntityDestroyedEvent
    {
        std::reference_wrapper<World> world;
        Entity entity;
    };
}
