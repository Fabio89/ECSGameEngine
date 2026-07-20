export module Components.BoundingBox;
import Assets.Mesh;
import Components.Transform;
import Core;
import Math;
import Physics;
import Serialization.Json;

export using ::TraceChannelFlags;

export struct BoundingBoxComponent
{
    TraceChannel channel{TraceChannelFlags::Default};
    Vec3 minLocal{};
    Vec3 maxLocal{};
    Vec3 minWorld{};
    Vec3 maxWorld{};
};

export namespace BoundingBoxUtils
{
    BoundingBoxComponent computeBoundingBox(const MeshData& mesh)
    {
        BoundingBoxComponent box;

        if (mesh.vertices.empty())
            return box;

        box.minLocal = mesh.vertices.front().pos;
        box.maxLocal = mesh.vertices.front().pos;

        for (const Vertex& vertex : mesh.vertices)
        {
            box.minLocal = Math::min(box.minLocal, vertex.pos);
            box.maxLocal = Math::max(box.maxLocal, vertex.pos);
        }

        return box;
    }
}

template<>
constexpr std::string_view getTypeName<BoundingBoxComponent>() { return "BoundingBoxComponent"; }

template<>
JsonObject serialize(const BoundingBoxComponent &component, Json::MemoryPoolAllocator<> &allocator)
{
    JsonObject json{Json::kObjectType};
    json.AddMember("channel", component.channel.toNumber(), allocator);
    json.AddMember("minLocal", Json::fromVec3(component.minLocal, allocator), allocator);
    json.AddMember("maxLocal", Json::fromVec3(component.maxLocal, allocator), allocator);
    return json;
}

template<>
BoundingBoxComponent deserialize(const JsonObject &data)
{
    const TraceChannel channel{
        static_cast<TraceChannelFlags>(Json::toNumber<TraceChannelFlagsType>(data, "channel").value_or(
            static_cast<TraceChannelFlagsType>(TraceChannelFlags::Default)))
    };
    const Vec3 minLocal = Json::toVec3(data, "minLocal").value_or(Vec3{});
    const Vec3 maxLocal = Json::toVec3(data, "maxLocal").value_or(Vec3{});
    log(std::format("Deserialized bounding box: ({}, {}, {}) ({}, {}, {})", minLocal.x, minLocal.y, minLocal.z,
                    maxLocal.x, maxLocal.y, maxLocal.z));
    return
    {
        .channel = channel,
        .minLocal = minLocal,
        .maxLocal = maxLocal
    };
}
