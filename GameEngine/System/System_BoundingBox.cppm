export module System.BoundingBox;
import Component.BoundingBox;
import Component.Transform;
import Math;
import Render.Model;
import System;
import std;

std::vector<LineVertex> generateAABBVertices(const Vec3& min, const Vec3& max)
{
    std::vector<Vec3> vertices =
    {
        // Bottom face (4 lines)
        {min.x, min.y, min.z}, {max.x, min.y, min.z},
        {max.x, min.y, min.z}, {max.x, max.y, min.z},
        {max.x, max.y, min.z}, {min.x, max.y, min.z},
        {min.x, max.y, min.z}, {min.x, min.y, min.z},

        // Top face (4 lines)
        {min.x, min.y, max.z}, {max.x, min.y, max.z},
        {max.x, min.y, max.z}, {max.x, max.y, max.z},
        {max.x, max.y, max.z}, {min.x, max.y, max.z},
        {min.x, max.y, max.z}, {min.x, min.y, max.z},

        // Vertical lines (4 lines)
        {min.x, min.y, min.z}, {min.x, min.y, max.z},
        {max.x, min.y, min.z}, {max.x, min.y, max.z},
        {max.x, max.y, min.z}, {max.x, max.y, max.z},
        {min.x, max.y, min.z}, {min.x, max.y, max.z}
    };

    std::vector<LineVertex> lineVertices;
    std::ranges::transform(vertices, std::back_inserter(lineVertices), [](const Vec3& pos)
    {
        return LineVertex{pos, Vec3{1.0f, 0.75f, 0.0f}};
    });
    return lineVertices;
}

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

export class System_BoundingBox final : public System
{
    void onComponentAdded(World& world, Entity entity, ComponentTypeId componentType) override
    {
        if (componentType == BoundingBoxComponent::typeId())
        {
            auto& aabb = world.editComponent<BoundingBoxComponent>(entity);
            const auto& transform = world.readComponent<TransformComponent>(entity);
            computeWorldCorners(aabb, transform);
            world.getRenderManager().setDebugRenderObject(entity, generateAABBVertices(aabb.minLocal, aabb.maxLocal));
        }
    }

    void onUpdate(World& world, [[maybe_unused]] Player& player, [[maybe_unused]] float deltaTime) override
    {
        for (auto&& [entity, aabb, transform] : world.view<BoundingBoxComponent, TransformComponent>())
        {
            computeWorldCorners(aabb, transform);
        }
    }

    static void computeWorldCorners(BoundingBoxComponent& aabb, const TransformComponent& transform)
    {
        const std::vector<Vec3>& corners = computeCorners(aabb);

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
