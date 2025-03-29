export module Component.Model;
import Core;
import AssetManager;
import Guid;
import Render.Model;
import Serialization;

export struct ModelComponent
{
    Guid mesh;
    Guid texture;
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
    json.AddMember("mesh", JsonObject{thing.mesh.toString().data(), allocator}, allocator);
    json.AddMember("texture", JsonObject{thing.texture.toString().data(), allocator}, allocator);
    return json;
}

template <>
ModelComponent deserialize(const JsonObject& serializedData)
{
    const Guid meshGuid = Guid::createFromString(serializedData.FindMember("mesh")->value.GetString());

    Guid textureGuid;
    if (const auto it = serializedData.FindMember("texture"); it != serializedData.MemberEnd())
    {
        if (std::string idString = it->value.GetString(); !idString.empty())
        {
            textureGuid = Guid::createFromString(std::move(idString));
        }
    }

    return {.mesh = meshGuid, .texture = textureGuid};
}
