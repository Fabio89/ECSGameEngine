export module System.LineRender;
import Component.LineRender;
import Component.Hierarchy;
import Component.Transform;
import Math;
import Render.Commands;
import System;

export class LineRenderSystem final : public System
{
    void onComponentAdded(World& world, Entity entity, TypeId componentType) override
    {
        if (componentType == getComponentType<LineRenderComponent>())
        {
            const auto& component = world.readComponent<LineRenderComponent>(entity);
            world.getRenderManager().addCommand(RenderCommands::AddLineObject{world.getHandle(), entity, component.vertices});
        }
    }

    void onEntityDestroyed(World& world, Entity entity) override
    {
        world.getRenderManager().addCommand(RenderCommands::RemoveLineObject{world.getHandle(), entity});
    }
};
