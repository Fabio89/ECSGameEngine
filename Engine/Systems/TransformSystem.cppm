export module System.Transform;
import Component.Hierarchy;
import Component.Transform;
import Engine.SystemManager;
import Engine;
import Math;
import Render.CommandProcessor;
import Render.Commands;
import World.Events;

void computeWorldTransform(World& world, Entity entity, Edit<TransformComponent>& transform)
{
    if (transform->runtimeData.calculatedThisFrame)
        return;

    transform->runtimeData.worldMatrix = TransformUtils::toMatrix(transform.get());

    if (const Entity parent = HierarchyUtils::getParent(world, entity); world.isValid(parent))
    {
        Edit<TransformComponent> parentTransform = world.editComponent<TransformComponent>(parent);
        computeWorldTransform(world, parent, parentTransform);
        transform->runtimeData.worldMatrix = parentTransform->runtimeData.worldMatrix * transform->runtimeData.worldMatrix;
    }

    transform->runtimeData.calculatedThisFrame = true;
}

void updateRenderTransform(World& world, RenderCommandQueue& renderQueue, Entity entity, Edit<TransformComponent>& transform)
{
    computeWorldTransform(world, entity, transform);
    renderQueue.addCommand(RenderCommands::SetTransform{world.getHandle(), entity, transform->runtimeData.worldMatrix});
}

namespace
{
    EventSubscription subscription;
}

void init(SystemContext& context)
{
    subscription += context.worlds.subscribe([&context](const WorldEvents::ComponentAdded& event)
    {
        World& world = context.worlds.get(event.world);
        if (event.componentType == getTypeId<TransformComponent>())
        {
            auto component = world.editComponent<TransformComponent>(event.entity);
            updateRenderTransform(world, context.renderCommands, event.entity, component);
        }
    });
}

void update(SystemContext& context, float)
{
    context.worlds.forEachWorld([&renderQueue = context.renderCommands](World& world)
    {
        for (auto&& [entity, transform] : world.query<Edit<TransformComponent>>())
        {
            updateRenderTransform(world, renderQueue, entity, transform);
        }

        for (auto&& [entity, transform] : world.query<Edit<TransformComponent>>())
        {
            transform->runtimeData.calculatedThisFrame = false;
        }
    });
}

void shutdown(SystemContext&)
{
    subscription.clear();
}

export namespace TransformSystem
{
    SystemCallbacks callbacks{.init = init, .update = update, .shutdown = shutdown};
}
