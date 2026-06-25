export module System.Model;
import Component.Model;
import Render.Commands;
import System;

export class ModelSystem final : public System
{
    void onComponentAdded(World& world, Entity entity, TypeId componentType) override
    {
        if (componentType == getComponentType<ModelComponent>())
        {
            const ModelComponent& model = world.readComponent<ModelComponent>(entity);
            world.getRenderManager().addCommand(RenderCommands::AddObject{entity, model.mesh, model.texture});
        }
    }
};
