export module System.BoundingBox;
import Component.BoundingBox;
import Component.Transform;
import Math;
import Render.Model;
import System;
import std;

std::vector<Vec3> generateAABBVertices(const Vec3& min, const Vec3& max)
{
    std::vector<Vec3> vertices;

    // Bottom face (4 lines)
    vertices.push_back({min.x, min.y, min.z});
    vertices.push_back({max.x, min.y, min.z});

    vertices.push_back({max.x, min.y, min.z});
    vertices.push_back({max.x, max.y, min.z});

    vertices.push_back({max.x, max.y, min.z});
    vertices.push_back({min.x, max.y, min.z});

    vertices.push_back({min.x, max.y, min.z});
    vertices.push_back({min.x, min.y, min.z});

    // Top face (4 lines)
    vertices.push_back({min.x, min.y, max.z});
    vertices.push_back({max.x, min.y, max.z});

    vertices.push_back({max.x, min.y, max.z});
    vertices.push_back({max.x, max.y, max.z});

    vertices.push_back({max.x, max.y, max.z});
    vertices.push_back({min.x, max.y, max.z});

    vertices.push_back({min.x, max.y, max.z});
    vertices.push_back({min.x, min.y, max.z});

    // Vertical lines (4 lines)
    vertices.push_back({min.x, min.y, min.z});
    vertices.push_back({min.x, min.y, max.z});

    vertices.push_back({max.x, min.y, min.z});
    vertices.push_back({max.x, min.y, max.z});

    vertices.push_back({max.x, max.y, min.z});
    vertices.push_back({max.x, max.y, max.z});

    vertices.push_back({min.x, max.y, min.z});
    vertices.push_back({min.x, max.y, max.z});

    return vertices;
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
public:
    void onUpdate(World& world, Player& player, float deltaTime) override
    {
        for (Entity entity : world.getEntitiesRange())
        {
            if (std::ranges::contains(world.getComponentTypesInEntity(entity), BoundingBoxComponent::typeId()))
            {
                auto& aabb = world.editComponent<BoundingBoxComponent>(entity);
                const auto& transform = world.readComponent<TransformComponent>(entity);

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

                world.getRenderManager().setDebugRenderObject(entity, generateAABBVertices(aabb.minWorld, aabb.maxWorld));
            }
        }
    }
};
