export module System.BoundingBox;
import Component.BoundingBox;
import Component.Transform;
import Math;
import System;

constexpr std::vector<Vec3> computeCorners(const BoundingBoxComponent& aabb)
{
    return
    {
        Vec3{aabb.minLocal.x, aabb.minLocal.y, aabb.minLocal.z},
        Vec3{aabb.maxLocal.x, aabb.minLocal.y, aabb.minLocal.z},
        Vec3{aabb.minLocal.x, aabb.maxLocal.y, aabb.minLocal.z},
        Vec3{aabb.maxLocal.x, aabb.maxLocal.y, aabb.minLocal.z},
        Vec3{aabb.minLocal.x, aabb.minLocal.y, aabb.maxLocal.z},
        Vec3{aabb.maxLocal.x, aabb.minLocal.y, aabb.maxLocal.z},
        Vec3{aabb.minLocal.x, aabb.maxLocal.y, aabb.maxLocal.z},
        Vec3{aabb.maxLocal.x, aabb.maxLocal.y, aabb.maxLocal.z}
    };
}

void computeWorldCorners(BoundingBoxComponent& aabb, const Mat4& worldTransform)
{
    const std::vector<Vec3>& corners = computeCorners(aabb);

    Vec3 minWorld{std::numeric_limits<float>::max()};
    Vec3 maxWorld{std::numeric_limits<float>::lowest()};
    
    for (int i = 0; i < 8; i++)
    {
        const Vec3 worldPos{worldTransform * Vec4{corners[i], 1.0f}};
        minWorld = Math::min(minWorld, worldPos);
        maxWorld = Math::max(maxWorld, worldPos);
    }

    aabb.minWorld = minWorld;
    aabb.maxWorld = maxWorld;
}

export class System_BoundingBox final : public System
{
    void onComponentAdded(World& world, Entity entity, ComponentTypeId componentType) override
    {
        if (componentType == getComponentType<BoundingBoxComponent>())
        {
            auto& aabb = world.editComponent<BoundingBoxComponent>(entity);
            const auto& transform = world.readComponent<TransformComponent>(entity);
            computeWorldCorners(aabb, transform.runtimeData.worldMatrix);
        }
    }

    void onUpdate(World& world, [[maybe_unused]] Player& player, [[maybe_unused]] float deltaTime) override
    {
        for (auto&& [entity, aabb, transform] : world.view<BoundingBoxComponent, TransformComponent>())
        {
            computeWorldCorners(aabb, transform.runtimeData.worldMatrix);
        }
    }
};
