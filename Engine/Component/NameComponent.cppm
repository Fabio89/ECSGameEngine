export module Component.Name;
import Core;
import Serialization;
import World;

export struct NameComponent
{
    std::string name{""};
};

template <>
struct TypeTraits<NameComponent>
{
    static constexpr auto name = "NameComponent";
};

export namespace NameUtils
{
    std::string_view getName(const World& world, Entity entity)
    {
        static constexpr const char* emptyName = "";
        return world.hasComponent<NameComponent>(entity) ? world.readComponent<NameComponent>(entity).name : emptyName;
    }
}

template <>
JsonObject serialize(const NameComponent& component, Json::MemoryPoolAllocator<>& allocator)
{
    JsonObject json{Json::kObjectType};
    json.AddMember("name", JsonObject{component.name.c_str(), allocator}, allocator);
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
