export module Systems.Hierarchy;
import Components.Hierarchy;
import Engine.SystemManager;
import EventBus;
import World;
import World.Events;

namespace
{
    EventSubscription subscription;

    void init(SystemContext& context)
    {
        subscription += context.worlds.subscribe([&](const WorldEvents::EntityDestroyed& event)
        {
            World& world = context.worlds.get(event.world);

            auto checkLink = [&](Entity& link)
            {
                if (link == event.entity)
                    link = {};
            };

            for (auto&& [entity, hierarchy] : world.query<Edit<HierarchyComponent>>())
            {
                checkLink(hierarchy->parent);
                checkLink(hierarchy->firstChild);
                checkLink(hierarchy->nextSibling);
                checkLink(hierarchy->previousSibling);
            }
        });
    }

    void shutdown(SystemContext&)
    {
        subscription.clear();
    }
}

export namespace HierarchySystem
{
    SystemCallbacks callbacks{.init = init, .shutdown = shutdown};
}