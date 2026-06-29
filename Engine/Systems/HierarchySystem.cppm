export module System.Hierarchy;
import Component.Hierarchy;
import Engine;
import EventBus;
import System;
import World;
import World.Events;

export class HierarchySystem final : public System
{
    void onEntityDestroyed(World& world, Entity entity) override
    {
        auto checkLink = [&](Entity& link)
        {
            if (link == entity)
                link = {};
        };

        for (auto&& [entity, hierarchy] : world.view<HierarchyComponent>())
        {
            checkLink(hierarchy.parent);
            checkLink(hierarchy.firstChild);
            checkLink(hierarchy.nextSibling);
            checkLink(hierarchy.previousSibling);
        }
    }
};