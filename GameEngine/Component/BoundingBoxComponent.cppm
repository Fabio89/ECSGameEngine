export module Component.BoundingBox;
import Component.Transform;
import Math;
import Serialization;
import System;
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

    return
    {
        .minLocal = minLocal,
        .maxLocal = maxLocal
    };
}

export class BoundingBoxSystem : public System
{
public:
    void onComponentAdded(World& world, Entity entity, ComponentTypeId componentType) override
    {
        if (componentType == BoundingBoxComponent::typeId)
        {
            updateBounds(world, entity);

            addUpdateFunction([&world, entity](float)
            {
                updateBounds(world, entity);
            });
        }
    }

private:
    static void updateBounds(World& world, Entity entity)
    {
        auto& aabb = world.editComponent<BoundingBoxComponent>(entity);
        const auto& transform = world.readComponent<TransformComponent>(entity);

        static const Vec3 corners[]
        {
            {aabb.minLocal.x, aabb.minLocal.y, aabb.minLocal.z},
            {aabb.maxLocal.x, aabb.minLocal.y, aabb.minLocal.z},
            {aabb.minLocal.x, aabb.maxLocal.y, aabb.minLocal.z},
            {aabb.maxLocal.x, aabb.maxLocal.y, aabb.minLocal.z},
            {aabb.minLocal.x, aabb.minLocal.y, aabb.maxLocal.z},
            {aabb.maxLocal.x, aabb.minLocal.y, aabb.maxLocal.z},
            {aabb.minLocal.x, aabb.maxLocal.y, aabb.maxLocal.z},
            {aabb.maxLocal.x, aabb.maxLocal.y, aabb.maxLocal.z}
        };

        Vec3 minWorld{std::numeric_limits<float>::max()};
        Vec3 maxWorld{std::numeric_limits<float>::lowest()};

        const Mat4 worldMatrix = TransformUtils::toMatrix(transform);

        for (int i = 0; i < 8; i++)
        {
            const Vec3 worldPos{worldMatrix * Vec4{corners[i], 1.0f}};
            minWorld = Math::min(minWorld, worldPos);
            maxWorld = Math::max(maxWorld, worldPos);
        }

        aabb.minWorld = minWorld;
        aabb.maxWorld = maxWorld;
    }
};
