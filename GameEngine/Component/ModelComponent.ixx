export module Engine.Component.Model;
import Engine.AssetManager;
import Engine.Render.Application;
import Engine.Render.Core;
import Engine.Core;
import Engine.Guid;
import Engine.Config;
import Engine.World;

export struct ModelComponent : Component<ModelComponent>
{
    const MeshAsset* mesh{nullptr};
    const TextureAsset* texture{nullptr};
};

template <>
ModelComponent Deserialize(const Json& serializedData)
{
    const Guid meshGuid{*serializedData.find("mesh")};
    const Guid textureGuid{*serializedData.find("texture")};
    
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
            world.getApplication().requestAddRenderObject
            ({
                .entity = entity,
                .mesh = model.mesh,
                .texture = model.texture
            });
        }
    }
};
