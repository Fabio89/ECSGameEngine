export module World.Events;
import Core;
import WorldHandle;

export namespace Engine
{
    struct SceneLoadedEvent
    {
        WorldHandle world;
    };

    struct EntityDestroyedEvent
    {
        WorldHandle world;
        Entity entity;
    };
}
