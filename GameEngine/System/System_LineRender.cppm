export module System.LineRender;
import Component.LineRender;
import Component.Parent;
import Component.Transform;
import Math;
import System;

export class System_LineRender final : public System
{
    void onComponentAdded(World& world, Entity entity, ComponentTypeId componentType) override
    {
        if (componentType == getComponentType<LineRenderComponent>())
        {
            const auto& component = world.readComponent<LineRenderComponent>(entity);
            world.getRenderManager().addCommand(RenderCommands::AddLineObject{entity, component.vertices});
        }
    }
};
