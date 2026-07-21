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
            if (hasFlag(proxy.flags, EntityProxyFlags::DestroyWithSource) && proxy.sourceWorld == sourceWorld)
                invalidated.push_back(entity);

        for (Entity entity : invalidated)
            proxyWorld.removeEntity(entity);
    }

    void init(SystemContext& context)
    {
        subscription += context.worlds.subscribe([&](const WorldEvents::ComponentAdded& event)
        {
            if (event.componentType == getTypeId<EntityProxyComponent>())
            {
                World& proxyWorld = context.worlds.get(event.world);
                const EntityProxyComponent& proxy = proxyWorld.readComponent<EntityProxyComponent>(event.entity);

                if (proxy.sourceWorld.isValid() && proxy.sourceEntity.isValid())
                {
                    const World& sourceWorld = context.worlds.get(proxy.sourceWorld);
                    EntityProxyUtils::snapToSourceEntity(proxyWorld, sourceWorld, event.entity, proxy);
                }
            }
        });

        subscription += context.worlds.subscribe([&](const WorldEvents::EntityDestroyed& event)
        {
            context.worlds.forEachWorld([sourceEntity = event.entity](World& world)
            {
                std::vector<Entity> invalidated;
                for (auto&& [entity, proxy] : world.query<EntityProxyComponent>())
                    if (hasFlag(proxy.flags, EntityProxyFlags::DestroyWithSource)
                        && proxy.sourceWorld == world.getHandle()
                        && proxy.sourceEntity == sourceEntity)
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
            for (auto&& [entity, proxy] : world.query<EntityProxyComponent>())
            {
                if (proxy.sourceWorld.isValid())
                {
                    const World& sourceWorld = context.worlds.get(proxy.sourceWorld);
                    EntityProxyUtils::snapToSourceEntity(world, sourceWorld, entity, proxy);
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