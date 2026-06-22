export module Component.Camera;
import Core;
import Math;
import Serialization;

export struct CameraComponent
{
    float fov{60.f};
    float nearPlane{0.1f};
    float farPlane{100.f};
    Mat4 projectionMatrix;
    Mat4 viewMatrix;
};

template <>
struct TypeTraits<CameraComponent>
{
    static constexpr auto name = "CameraComponent";
};

template<>
JsonObject serialize(const CameraComponent& component, Json::MemoryPoolAllocator<>& allocator)
{
    JsonObject json{Json::kObjectType};
    json.AddMember("fov", component.fov, allocator);
    return json;
}

template <>
CameraComponent deserialize(const JsonObject& data)
{
    CameraComponent camera;
    if (const auto it = data.FindMember("fov"); it != data.MemberEnd())
    {
        camera.fov = it->value.GetFloat();
    }

    return camera;
}