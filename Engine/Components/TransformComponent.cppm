export module Components.Transform;
export import Math;
import Components.Hierarchy;
import Properties;
import Serialization.Json;
import World;

export struct TransformComponent
{
    Vec3 position{};
    Quat rotation{};
    Vec3 scale{1.f};
};

export struct RuntimeTransformComponent
{
    Mat4 worldMatrix{};
};

export namespace TransformUtils
{
    Vec3 forward(const TransformComponent& transform);

    Vec3 right(const TransformComponent& transform);

    Vec3 up(const TransformComponent& transform);

    Mat4 toMatrix(const TransformComponent& transform);

    TransformComponent getWorldTransform(const World& world, Entity entity);

    void setWorldTransform(World& world, Entity entity, const TransformComponent& worldTransform);

    void editWorldTransform(World& world, Entity entity, const std::function<void(TransformComponent& worldTransform)>& editFunc);

    void forceApplyTransform(World& world, Entity entity);
}

template<>
constexpr std::string_view getTypeName<TransformComponent>() { return "TransformComponent"; }

template<>
struct TypeProperties<TransformComponent>
{
    static constexpr std::tuple list{
        makeProperty("position", &TransformComponent::position),
        makeProperty("rotation", &TransformComponent::rotation),
        makeProperty("scale", &TransformComponent::scale)
    };
};

template<>
JsonObject serialize(const TransformComponent& component, Json::MemoryPoolAllocator<>& allocator)
{
    JsonObject json{Json::kObjectType};
    json.AddMember("position", Json::fromVec3(component.position, allocator), allocator);
    json.AddMember("rotation", Json::fromQuat(component.rotation, allocator), allocator);
    json.AddMember("scale", Json::fromVec3(component.scale, allocator), allocator);
    return json;
}

template<>
TransformComponent deserialize(const JsonObject& data)
{
    const Vec3 position = Json::toVec3(data, "position").value_or(Vec3{});
    const Quat rotation = Json::toQuat(data, "rotation").value_or(Quat{});
    const Vec3 scale = Json::toVec3(data, "scale").value_or(Vec3{});

    return
    {
        .position = position,
        .rotation = rotation,
        .scale = scale
    };
}

template<>
constexpr std::string_view getTypeName<RuntimeTransformComponent>() { return "RuntimeTransformComponent"; }

template<>
struct TypeProperties<RuntimeTransformComponent>
{
    static constexpr std::tuple list{
        makeProperty("worldMatrix", &RuntimeTransformComponent::worldMatrix)
    };
};