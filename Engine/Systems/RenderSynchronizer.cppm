export module Systems.RenderSynchronizer;
import Components.LineRender;
import Components.Model;
import Engine.SystemManager;
import EventBus;
import Math;
import Render.CommandProcessor;
import Render.Commands;
import World.Events;

import Engine;

namespace
{
    EventSubscription subscription;
}

Mat4 getWorldTransform(const World& world, Entity entity)
{
    if (world.hasComponent<RuntimeTransformComponent>(entity))
        return world.readComponent<RuntimeTransformComponent>(entity).worldMatrix;
    static constexpr Mat4 identity{1};
    return identity;
}

void init(SystemContext& context)
{
    subscription += context.worlds.subscribe([&](const WorldEvents::WorldCreated& event)
    {
        context.renderCommands.addCommand(RenderCommands::AddWorld{.world = event.world});
    });

    subscription += context.worlds.subscribe([&](const WorldEvents::WorldDestroyed& event)
    {
         context.renderCommands.addCommand(RenderCommands::RemoveWorld{.world = event.world});
    });

    subscription += context.worlds.subscribe([&](const WorldEvents::WorldCleared& event)
    {
        context.renderCommands.addCommand(RenderCommands::ClearRenderObjects{.world = event.world});
    });

    subscription += context.worlds.subscribe([&](const WorldEvents::ComponentAdded& event)
    {
        const World& world = context.worlds.get(event.world);
        if (event.componentType == getTypeId<LineRenderComponent>())
        {
            const auto& component = world.readComponent<LineRenderComponent>(event.entity);
            context.renderCommands.addCommand(RenderCommands::AddLineObject{event.world, event.entity, component.vertices, getWorldTransform(world, event.entity)});
        }
        else if (event.componentType == getTypeId<ModelComponent>())
        {
            const auto& component = world.readComponent<ModelComponent>(event.entity);
            context.renderCommands.addCommand(RenderCommands::AddObject{event.world, event.entity, component.mesh, component.texture, getWorldTransform(world, event.entity), component.layer});
        }
    });

    subscription += context.worlds.subscribe([&](const WorldEvents::EntityDestroyed& event)
    {
         context.renderCommands.addCommand(RenderCommands::RemoveObject{event.world, event.entity});
         context.renderCommands.addCommand(RenderCommands::RemoveLineObject{event.world, event.entity});
    });
}

void update(SystemContext& context, float deltaTime)
{
    context.worlds.forEachWorld([&](World& world)
    {
        for (const Entity entity : world.getMarked<RuntimeTransformComponent>())
        {
            const auto& transform = world.readComponent<RuntimeTransformComponent>(entity);
            context.renderCommands.addCommand(RenderCommands::SetTransform{world.getHandle(), entity, transform.worldMatrix});
        }
    });
}

void shutdown(SystemContext&)
{
    subscription.clear();
}

export namespace RenderSynchronizer
{
    SystemCallbacks callbacks{.init = init, .update = update, .shutdown = shutdown};
}
