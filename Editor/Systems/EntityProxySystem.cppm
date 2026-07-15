export module Systems.EntityProxy;
import Components.EntityProxy;
import Components.Transform;
import Engine.SystemManager;
import World.Events;

namespace
{
    EventSubscription subscription;

    void destroyEntitiesFollowingWorld(World& proxyWorld, WorldHandle sourceWorld)
    {
        std::vector<Entity> invalidated;
        for (auto&& [entity, proxy] : proxyWorld.query<EntityProxyComponent>())
            if (proxy.sourceWorld == sourceWorld)
                invalidated.push_back(entity);

        for (Entity entity : invalidated)
            proxyWorld.removeEntity(entity);
    }

    void init(SystemContext& context)
    {
        subscription += context.worlds.subscribe([&](const WorldEvents::EntityDestroyed& event)
        {
            context.worlds.forEachWorld([sourceEntity = event.entity](World& world)
            {
                std::vector<Entity> invalidated;
                for (auto&& [entity, proxy] : world.query<EntityProxyComponent>())
                    if (proxy.sourceWorld == world.getHandle() && proxy.sourceEntity == sourceEntity)
                        invalidated.push_back(entity);

                for (Entity entity : invalidated)
                    world.removeEntity(entity);
            });
        });

        subscription += context.worlds.subscribe([&](const WorldEvents::WorldCleared& event)
        {
            context.worlds.forEachWorld([clearedWorld = event.world](World& world)
            {
                destroyEntitiesFollowingWorld(world, clearedWorld);
            });
        });

        subscription += context.worlds.subscribe([&](const WorldEvents::WorldDestroyed& event)
        {
            context.worlds.forEachWorld([clearedWorld = event.world](World& world)
            {
                destroyEntitiesFollowingWorld(world, clearedWorld);
            });
        });
    }

    void update(SystemContext& context, float)
    {
        context.worlds.forEachWorld([&context](World& world)
        {
            for (auto&& [entity, proxy, transform] : world.query<EntityProxyComponent, Edit<RuntimeTransformComponent>>())
            {
                if (!proxy.sourceEntity.isValid() || !proxy.sourceWorld.isValid())
                    continue;

                const World& sourceWorld = context.worlds.get(proxy.sourceWorld);
                if (sourceWorld.hasComponent<RuntimeTransformComponent>(proxy.sourceEntity))
                {
                    const auto& sourceTransform = sourceWorld.readComponent<RuntimeTransformComponent>(proxy.sourceEntity);
                    transform->worldMatrix = sourceTransform.worldMatrix;
                }
            }
        });
    }

    void shutdown(SystemContext&)
    {
        subscription.clear();
    }
}

export namespace EntityProxySystem
{
    SystemCallbacks callbacks{.init = init, .update = update, .shutdown = shutdown};
}