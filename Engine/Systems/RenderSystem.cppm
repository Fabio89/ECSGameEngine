export module System.Render;
import Component.Model;
import Component.Transform;
import Render.Commands;
import System;

export class RenderSystem final : public System
{
    void onComponentAdded(World& world, Entity entity, TypeId componentType) override
    {
        if (componentType == getComponentType<ModelComponent>())
        {
            const auto& component = world.readComponent<ModelComponent>(entity);
            world.getRenderManager().addCommand(RenderCommands::AddObject{entity, component.mesh, component.texture});
        } else if (componentType == getComponentType<TransformComponent>())
        {
            const auto& component = world.readComponent<TransformComponent>(entity);
            updateRenderTransform(world, entity, component);
        }
    }

    void onUpdate(World& world, [[maybe_unused]] Player& player, [[maybe_unused]] float deltaTime) override
    {
        for (auto&& [entity, transform] : world.view<TransformComponent>())
        {
            updateRenderTransform(world, entity, transform);
        }
    }

    void onEntityDestroyed(World& world, Entity entity) override
    {
        world.getRenderManager().addCommand(RenderCommands::RemoveObject{entity});
    }

    static void updateRenderTransform(World& world, Entity entity, const TransformComponent& transform)
    {
        world.getRenderManager().addCommand(RenderCommands::SetTransform{entity, TransformUtils::toMatrix(transform)});
    }
};
