export module Engine.Component.Model;
import Engine.AssetManager;
import Engine.ComponentRegistry;
import Engine.Render.Application;
import Engine.Render.Core;
import Engine.Core;
import Engine.Guid;
import Engine.Config;
import Engine.World;

export struct ModelComponent : Component<ModelComponent>
{
    const MeshAsset* mesh{};
    const TextureAsset* texture{};
};

//ComponentRegistry::Entry<ModelComponent> reg{"Name"};

template <>
ModelComponent deserialize(const Json& serializedData)
{
    const Guid meshGuid = Guid::createFromString(*serializedData.find("mesh"));
    const Guid textureGuid = Guid::createFromString(*serializedData.find("texture"));
    
    return
    {
        .mesh = AssetManager::findAsset<MeshAsset>(meshGuid),
        .texture = AssetManager::findAsset<TextureAsset>(textureGuid)
    };
}

export class ModelSystem : public System
{
public:
    void onComponentAdded(World& world, Entity entity, ComponentTypeId componentType) override
    {
        if (componentType == ModelComponent::typeId)
        {
            const ModelComponent& model = world.readComponent<ModelComponent>(entity);
            world.getRenderManager().addRenderObject(entity, model.mesh, model.texture);
        }
    }
};
