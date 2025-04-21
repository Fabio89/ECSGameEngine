export module Component.PersistentId;
import Core;
import Guid;
import Serialization;

export struct PersistentIdComponent
{
    Guid id;
};

template <>
struct TypeTraits<PersistentIdComponent>
{
    static constexpr auto name = "PersistentIdComponent";
};

template <>
JsonObject serialize(const PersistentIdComponent& component, Json::MemoryPoolAllocator<>& allocator)
{
    JsonObject json{Json::kObjectType};
    json.AddMember("id", JsonObject{component.id.toString().data(), allocator}, allocator);
    return json;
}

template <>
PersistentIdComponent deserialize(const JsonObject& data)
{
    const Guid id = Guid::createFromString(data.FindMember("id")->value.GetString());
    return {.id = id};
}

namespace PersistentIdUtils
{
    std::unordered_map<Guid, Entity> uuidToEntityMap;

    export void registerEntity(Entity entity, const Guid& uuid)
    {
        uuidToEntityMap.try_emplace(uuid, entity);
    }

    export Entity getEntity(const Guid& uuid)
    {
        const auto it = uuidToEntityMap.find(uuid);
        return it != uuidToEntityMap.end() ? it->second : invalidId();
    }
}
