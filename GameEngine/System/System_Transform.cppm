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
            auto& component = world.editComponent<TransformComponent>(entity);
            updateRenderTransform(world, entity, component);
        }
    }

    void onUpdate(World& world, [[maybe_unused]] Player& player, [[maybe_unused]] float deltaTime) override
    {
        for (auto&& [entity, transform] : world.view<TransformComponent>())
        {
            updateRenderTransform(world, entity, transform);
        }

        for (auto&& [entity, transform] : world.view<TransformComponent>())
        {
            transform.runtimeData.calculatedThisFrame = false;
        }
    }

    static void updateRenderTransform(World& world, Entity entity, TransformComponent& transform)
    {
        computeWorldTransform(world, entity, transform);
         
        world.getRenderManager().addCommand(RenderCommands::SetTransform{entity, transform.runtimeData.worldMatrix});
    }

    static void computeWorldTransform(World& world, Entity entity, TransformComponent& transform)
    {
        if (transform.runtimeData.calculatedThisFrame)
            return;
        
        transform.runtimeData.worldMatrix = TransformUtils::toMatrix(transform);
        
        if (world.hasComponent<ParentComponent>(entity))
        {
            const auto& [parentEntity] = world.readComponent<ParentComponent>(entity);
            TransformComponent& parentTransform = world.editComponent<TransformComponent>(parentEntity);
            if (parentEntity != invalidId())
            {
                computeWorldTransform(world, parentEntity, parentTransform);
                transform.runtimeData.worldMatrix = parentTransform.runtimeData.worldMatrix * transform.runtimeData.worldMatrix;
            }
        }
        transform.runtimeData.calculatedThisFrame = true;
    }
};
