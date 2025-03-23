export module Component.Model;
import AssetManager;
import Guid;
import Render.Model;
import Serialization;
import System;
import World;

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
JsonObject serialize(const ModelComponent& thing, Json::MemoryPoolAllocator<>& allocator)
{
    JsonObject json{Json::kObjectType};
    json.AddMember("mesh", JsonObject{thing.mesh ? thing.mesh->getGuid().toString().data() : "", allocator}, allocator);
    json.AddMember("texture", JsonObject{thing.texture ? thing.texture->getGuid().toString().data() : "", allocator}, allocator);
    return json;
}

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
