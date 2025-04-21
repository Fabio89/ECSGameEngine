export module Component.Parent;
import Core;
import Guid;
import Serialization;

export struct ParentComponent
{
    Guid parent;
};

template <>
struct TypeTraits<ParentComponent>
{
    static constexpr auto name = "ParentComponent";
};

template <>
JsonObject serialize(const ParentComponent& component, Json::MemoryPoolAllocator<>& allocator)
{
    JsonObject json{Json::kObjectType};
    json.AddMember("parent", JsonObject{component.parent.toString().data(), allocator}, allocator);
    return json;
}

template <>
ParentComponent deserialize(const JsonObject& data)
{
    const Guid parent = Guid::createFromString(data.FindMember("parent")->value.GetString());
    return {.parent = parent};
}
