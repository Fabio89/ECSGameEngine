export module System.Render;
import Component.Model;
import Component.Transform;
import System;

export class System_Render final : public System
{
    void onComponentAdded(World& world, Entity entity, ComponentTypeId componentType) override
    {
        if (componentType == getComponentType<ModelComponent>())
        {
            const auto& component = world.readComponent<ModelComponent>(entity);
            world.getRenderManager().addCommand(RenderCommands::AddObject{entity, component.mesh, component.texture});
        }
        else if (componentType == getComponentType<TransformComponent>())
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

    static void updateRenderTransform(World& world, Entity entity, const TransformComponent& transform)
    {
        world.getRenderManager().addCommand(RenderCommands::SetTransform{entity, transform.position, transform.rotation, transform.scale});
    }
};
