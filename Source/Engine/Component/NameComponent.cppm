export module Component.Name;
import Core;
import Properties;
import Serialization;
import World;

export struct NameComponent
{
    std::string name{""};
};

template<>
constexpr std::string_view getTypeName<NameComponent>() { return "NameComponent"; }

template<>
struct TypeProperties<NameComponent>
{
    static constexpr std::tuple list{
        makeProperty("name", &NameComponent::name)
    };
};

export namespace NameUtils
{
    std::string_view getName(const World &world, Entity entity)
    {
        static constexpr const char *emptyName = "";
        return world.hasComponent<NameComponent>(entity) ? world.readComponent<NameComponent>(entity).name.c_str() : emptyName;
    }
}

template<>
JsonObject serialize(const NameComponent &component, Json::MemoryPoolAllocator<> &allocator)
{
    JsonObject json{Json::kObjectType};
    json.AddMember("name", JsonObject{component.name.c_str(), allocator}, allocator);
    return json;
}

template<>
NameComponent deserialize(const JsonObject &data)
{
    NameComponent ret;
    if (auto it = data.FindMember("name"); it != data.MemberEnd())
    {
        ret.name = it->value.GetString();
    }
    return ret;
}
