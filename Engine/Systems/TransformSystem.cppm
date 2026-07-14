export module System.Transform;
import Engine.SystemManager;

void init(SystemContext& context);
void update(SystemContext& context, float);
void shutdown(SystemContext&);

export namespace TransformSystem
{
    void ensureRuntimeTransform(World& world, Entity entity);

    SystemCallbacks callbacks{.init = init, .update = update, .shutdown = shutdown};
}
