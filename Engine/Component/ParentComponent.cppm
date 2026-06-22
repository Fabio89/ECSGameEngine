export module Component.Parent;
import Component.PersistentId;
import Core;
import Guid;
import Serialization;

export struct ParentComponent
{
    Entity parent;
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
    
    json.AddMember("parent", JsonObject{PersistentIdUtils::getUuid(component.parent).toString().data(), allocator}, allocator);
    return json;
}

template <>
ParentComponent deserialize(const JsonObject& data)
{
    const Guid parent = Guid::createFromString(data.FindMember("parent")->value.GetString());
    return {.parent = PersistentIdUtils::getEntity(parent)};
}
