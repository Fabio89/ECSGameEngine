export module System.Model;
import Component.Model;
import System;

export class System_Model final : public System
{
    void onComponentAdded(World& world, Entity entity, ComponentTypeId componentType) override
    {
        if (componentType == getComponentType<ModelComponent>())
        {
            const ModelComponent& model = world.readComponent<ModelComponent>(entity);
            world.getRenderManager().addCommand(RenderCommands::AddObject{entity, model.mesh, model.texture});
        }
    }
};
