export module System.LineRender;
import Component.LineRender;
import Component.Transform;
import System;

export class System_LineRender final : public System
{
    void onComponentAdded(World& world, Entity entity, ComponentTypeId componentType) override
    {
        if (componentType == getComponentType<LineRenderComponent>())
        {
            const auto& component = world.readComponent<LineRenderComponent>(entity);
            world.getRenderManager().addCommand(RenderCommands::AddLineObject{entity, component.vertices});
            if (component.parent != invalidId() && world.hasComponent<TransformComponent>(entity) && world.hasComponent<TransformComponent>(component.parent))
            {
                const auto& transform = world.editComponent<TransformComponent>(entity) = world.readComponent<TransformComponent>(component.parent);
                world.getRenderManager().addCommand(RenderCommands::SetTransform{entity, transform.position, transform.rotation, transform.scale});
            }
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
