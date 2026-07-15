export module World.Events;
export import EventBus;
export import WorldHandle;
import Core;

export namespace WorldEvents
{
    struct WorldCreated
    {
        WorldHandle world;
    };

    struct WorldDestroyed
    {
        WorldHandle world;
    };

    struct WorldCleared
    {
        WorldHandle world;
    };

    struct EntityCreated
    {
        WorldHandle world;
        Entity entity;
    };

    struct EntityDestroyed
    {
        WorldHandle world;
        Entity entity;
    };

    struct ComponentAdded
    {
        WorldHandle world;
        Entity entity;
        TypeId componentType;
    };

    struct ComponentRemoved
    {
        WorldHandle world;
        Entity entity;
        TypeId componentType;
    };
}
