export module Engine:Component.Name;
import :ComponentRegistry;
import :Ecs;
import :World;
import Serialization;

export struct NameComponent : Component<NameComponent>
{
    std::string name{""};
};

template <>
struct TypeTraits<NameComponent>
{
    static constexpr auto name = "NameComponent";
};

template <>
JsonObject serialize(const NameComponent& thing, Json::MemoryPoolAllocator<>& allocator)
{
    JsonObject json{Json::kObjectType};
    json.AddMember("name", JsonObject{thing.name.c_str(), allocator}, allocator);
    return json;
}

template <>
NameComponent deserialize(const JsonObject& data)
{
    NameComponent ret;
    if (auto it = data.FindMember("name"); it != data.MemberEnd())
    {
        ret.name = it->value.GetString();
    }
    return ret;
}
