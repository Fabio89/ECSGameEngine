export module System.LineRender;
import Component.LineRender;
import Component.Transform;
import System;

export class System_LineRender final : public System
{
    void onComponentAdded(World& world, Entity entity, ComponentTypeId componentType) override
    {
        if (componentType == LineRenderComponent::typeId())
        {
            const auto& component = world.readComponent<LineRenderComponent>(entity);
            world.getRenderManager().setLineRenderObject(entity, component.vertices);
        }
    }

    void onUpdate(World& world, [[maybe_unused]] Player& player, [[maybe_unused]] float deltaTime) override
    {
        for (auto&& [entity, lineRender, transform] : world.view<LineRenderComponent, TransformComponent>())
        {
            if (lineRender.parent != invalidId())
                transform = world.readComponent<TransformComponent>(lineRender.parent);
        }
    }
};