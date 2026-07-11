export module System.PersistentId;
import Component.PersistentId;
import Engine.SystemManager;
import World.Events;

namespace
{
    EventSubscription subscription;
}

void init(SystemContext& context)
{
    subscription += context.worlds.subscribe([&context](const WorldEvents::ComponentAdded& event)
    {
        World& world = context.worlds.get(event.world);
        if (event.componentType == getTypeId<PersistentIdComponent>())
        {
            PersistentIdUtils::registerEntity(event.entity, world.readComponent<PersistentIdComponent>(event.entity).id);
        }
    });
}

void shutdown(SystemContext&)
{
    subscription.clear();
}

export namespace PersistentIdSystem
{
    SystemCallbacks callbacks{.init = init, .shutdown = shutdown};
}