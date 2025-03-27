export module System.Transform;
import Component.Transform;
import System;

export class System_Transform final : public System
{
    void onComponentAdded(World& world, Entity entity, ComponentTypeId componentType) override
    {
        if (componentType == TransformComponent::typeId())
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
        world.getRenderManager().setRenderObjectTransform(entity, transform.position, transform.rotation, transform.scale);
    }
};