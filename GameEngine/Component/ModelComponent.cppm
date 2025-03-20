export module Engine:Component.Model;
import :AssetManager;
import :ComponentRegistry;
import :Render.RenderManager;
import :Render.Model;
import :Core;
import :Guid;
import :Serialization;
import :System;
import :World;

export struct ModelComponent : Component<ModelComponent>
{
    const MeshAsset* mesh{};
    const TextureAsset* texture{};
};

template <>
struct TypeTraits<ModelComponent>
{
    static constexpr auto name = "ModelComponent";
};

template <>
ModelComponent deserialize(const JsonObject& serializedData)
{
    const Guid meshGuid = Guid::createFromString(serializedData.FindMember("mesh")->value.GetString());
    const Guid textureGuid = Guid::createFromString(serializedData.FindMember("texture")->value.GetString());

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
