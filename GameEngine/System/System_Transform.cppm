export module System.Transform;
import Component.Transform;
import System;

export class System_Transform final : public System
{
public:
    void onComponentAdded(World& world, Entity entity, ComponentTypeId componentType) override
    {
        if (componentType == TransformComponent::typeId())
        {
            updateRenderTransform(world, entity);

            addUpdateFunction([&world, entity](float)
            {
                updateRenderTransform(world, entity);
            });
        }
    }

private:
    static void updateRenderTransform(World& world, Entity entity)
    {
        const auto& component = world.readComponent<TransformComponent>(entity);
        world.getRenderManager().setRenderObjectTransform(entity, component.position, component.rotation, component.scale);
    }
};