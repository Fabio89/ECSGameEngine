export module Scene.Events;
import WorldHandle;

export namespace SceneEvents
{
    struct Loaded
    {
        WorldHandle world;
    };

    struct Unloaded
    {
        WorldHandle world;
    };
}
