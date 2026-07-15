export module Components.Camera;
import Core;
import Math;
import Properties;
import Serialization.Json;
import Engine.Camera;
import Engine.Viewport;
import World;

export struct CameraComponent
{
    float fov{60.f};
    float nearPlane{0.1f};
    float farPlane{100.f};
};

export namespace CameraUtils
{
    Camera toRenderCamera(const World& world, ViewportId viewportId, Entity cameraEntity);
}

template<>
constexpr std::string_view getTypeName<CameraComponent>() { return "CameraComponent"; }

template<>
struct TypeProperties<CameraComponent>
{
    static constexpr std::tuple list{
        makeProperty("fov", &CameraComponent::fov),
        makeProperty("nearPlane", &CameraComponent::nearPlane),
        makeProperty("farPlane", &CameraComponent::farPlane)
    };
};

template<>
JsonObject serialize(const CameraComponent &component, Json::MemoryPoolAllocator<> &allocator)
{
    JsonObject json{Json::kObjectType};
    json.AddMember("fov", component.fov, allocator);
    return json;
}

template<>
CameraComponent deserialize(const JsonObject &data)
{
    CameraComponent camera;
    if (const auto it = data.FindMember("fov"); it != data.MemberEnd())
    {
        camera.fov = it->value.GetFloat();
    }

    return camera;
}
