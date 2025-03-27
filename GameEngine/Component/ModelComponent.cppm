export module Component.Model;
import AssetManager;
import Guid;
import Render.Model;
import Serialization;
import World;
import std;

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
    const MeshAsset* mesh = AssetManager::findAsset<MeshAsset>(meshGuid);

    const TextureAsset* texture = nullptr;
    if (auto it = serializedData.FindMember("texture"); it != serializedData.MemberEnd())
    {
        if (std::string idString = it->value.GetString(); !idString.empty())
        {
            const Guid textureGuid = Guid::createFromString(std::move(idString));
            texture = AssetManager::findAsset<TextureAsset>(textureGuid);
        }
    }

    return {.mesh = mesh, .texture = texture};
}
