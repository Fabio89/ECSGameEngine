export module Component.Transform;
import Core;
import Math;
import Serialization;

export struct TransformComponent
{
    Vec3 position;
    Quat rotation;
    float scale{1.f};
};

export namespace TransformUtils
{
    __declspec(dllexport)
    Vec3 forward(const TransformComponent& transform) { return Math::normalize(Math::rotate(transform.rotation, forwardVector())); }

    __declspec(dllexport)
    Vec3 right(const TransformComponent& transform) { return Math::normalize(Math::rotate(transform.rotation, rightVector())); }

    __declspec(dllexport)
    Vec3 up(const TransformComponent& transform) { return Math::normalize(Math::rotate(transform.rotation, upVector())); }

    Mat4 toMatrix(const TransformComponent& transform)
    {
        return Math::translate(Mat4{1.0f}, transform.position) * Math::mat4_cast(transform.rotation) * Math::scale(Mat4{1.0f}, Vec3{transform.scale});
    }
}

template <>
struct TypeTraits<TransformComponent>
{
    static constexpr auto name = "TransformComponent";
};

template <>
JsonObject serialize(const TransformComponent& component, Json::MemoryPoolAllocator<>& allocator)
{
    JsonObject json{Json::kObjectType};
    json.AddMember("position", Json::fromVec3(component.position, allocator), allocator);
    json.AddMember("rotation", Json::fromQuat(component.rotation, allocator), allocator);
    json.AddMember("scale", component.scale, allocator);
    return json;
}

template <>
TransformComponent deserialize(const JsonObject& data)
{
    const Vec3 position = Json::toVec3(data, "position").value_or(Vec3{});
    const Quat rotation = Json::toQuat(data, "rotation").value_or(Quat{});

    float scale{1.f};
    if (const auto it = data.FindMember("scale"); it != data.MemberEnd())
    {
        scale = it->value.GetFloat();
    }

    return
    {
        .position = position,
        .rotation = rotation,
        .scale = scale
    };
}
