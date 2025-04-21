export module System.Transform;
import Component.Parent;
import Component.Transform;
import Math;
import System;

export class System_Transform final : public System
{
    void onComponentAdded(World& world, Entity entity, ComponentTypeId componentType) override
    {
        if (componentType == getComponentType<TransformComponent>())
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
        Mat4 worldTransform = TransformUtils::toMatrix(transform);

        if (world.hasComponent<ParentComponent>(entity))
        {
            const ParentComponent& parentComponent = world.readComponent<ParentComponent>(entity);
            if (parentComponent.parent != invalidId())
            {
                const TransformComponent& parentTransform = world.readComponent<TransformComponent>(parentComponent.parent);
                worldTransform = TransformUtils::toMatrix(parentTransform) * worldTransform;
            }
        }
        
        world.getRenderManager().addCommand(RenderCommands::SetTransform{entity, worldTransform});
    }
};
