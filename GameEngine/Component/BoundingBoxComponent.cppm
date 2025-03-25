export module Component.BoundingBox;
import Component.Transform;
import Math;
import Serialization;
import World;
import std;

export struct BoundingBoxComponent : Component<BoundingBoxComponent>
{
    Vec3 minLocal;
    Vec3 maxLocal;
    Vec3 minWorld;
    Vec3 maxWorld;
};

template <>
struct TypeTraits<BoundingBoxComponent>
{
    static constexpr auto name = "BoundingBoxComponent";
};

template <>
JsonObject serialize(const BoundingBoxComponent& component, Json::MemoryPoolAllocator<>& allocator)
{
    JsonObject json{Json::kObjectType};
    json.AddMember("minLocal", Json::fromVec3(component.minLocal, allocator), allocator);
    json.AddMember("maxLocal", Json::fromVec3(component.maxLocal, allocator), allocator);
    return json;
}

template <>
BoundingBoxComponent deserialize(const JsonObject& data)
{
    const Vec3 minLocal = Json::toVec3(data, "minLocal").value_or(Vec3{});
    const Vec3 maxLocal = Json::toVec3(data, "maxLocal").value_or(Vec3{});
    log(std::format("Deserialized bounding box: ({}, {}, {}) ({}, {}, {})", minLocal.x, minLocal.y, minLocal.z, maxLocal.x, maxLocal.y, maxLocal.z));
    return
    {
        .minLocal = minLocal,
        .maxLocal = maxLocal
    };
}
