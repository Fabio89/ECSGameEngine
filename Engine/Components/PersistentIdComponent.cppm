export module Component.PersistentId;
import Core;
import Guid;
import Serialization.Json;

export struct PersistentIdComponent
{
    Guid id{};
};

template<>
constexpr std::string_view getTypeName<PersistentIdComponent>() { return "PersistentIdComponent"; }

template<>
JsonObject serialize(const PersistentIdComponent &component, Json::MemoryPoolAllocator<> &allocator)
{
    JsonObject json{Json::kObjectType};
    json.AddMember("id", JsonObject{component.id.toString().data(), allocator}, allocator);
    return json;
}

template<>
PersistentIdComponent deserialize(const JsonObject &data)
{
    const Guid id = Guid::createFromString(data.FindMember("id")->value.GetString());
    return {.id = id};
}

namespace PersistentIdUtils
{
    std::unordered_map<Guid, Entity> uuidToEntityMap;
    std::unordered_map<Entity, Guid> entityToUuidMap;

    export void registerEntity(Entity entity, const Guid &uuid)
    {
        uuidToEntityMap.try_emplace(uuid, entity);
        entityToUuidMap.try_emplace(entity, uuid);
    }

    export Entity getEntity(const Guid &uuid)
    {
        const auto it = uuidToEntityMap.find(uuid);
        return it != uuidToEntityMap.end() ? it->second : Entity{};
    }

    export const Guid& getUuid(Entity entity)
    {
        static constexpr Guid invalidUuid{};

        const auto it = entityToUuidMap.find(entity);
        return it != entityToUuidMap.end() ? it->second : invalidUuid;
    }
}
