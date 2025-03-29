module Editor.Gizmos;
import Component.LineRender;
import Component.BoundingBox;
import Component.Name;
import Component.Model;
import Component.Render;
import Component.Tags;
import Component.Transform;
import Guid;
import Math;
import Render.Model;
import Render.Primitives;

Entity createGizmo(World& world, MeshData&& mesh)
{
    Entity gizmo = world.createEntity();
    world.addComponent<NameComponent>(gizmo, "Gizmo");
    world.addComponent<TagsComponent>(gizmo, {{Tag::notEditable}});
    world.addComponent<TransformComponent>(gizmo);

    const Guid meshGuid = Guid::createRandom();
    world.getRenderManager().addCommand(RenderCommands::AddMesh{meshGuid, std::forward<MeshData>(mesh)});
    world.addComponent<ModelComponent>(gizmo, {.mesh = meshGuid});
    return gizmo;
}

Entity createGizmo(World& world, std::vector<LineVertex>&& vertices)
{
    Entity gizmo = world.createEntity();
    world.addComponent<NameComponent>(gizmo, "Gizmo");
    world.addComponent<TagsComponent>(gizmo, {{Tag::notEditable}});
    world.addComponent<TransformComponent>(gizmo);
    world.addComponent<LineRenderComponent>(gizmo, {.vertices = std::move(vertices)});
    world.getRenderManager().addCommand(RenderCommands::SetObjectVisibility{gizmo, false});
    return gizmo;
}

Entity EditorUtils::createTranslationGizmo(World& world)
{
    return createGizmo
    (
        world,
        {
            // X-axis line (red)
            LineVertex{{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}}, // Start of red line
            LineVertex{{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}}, // End of red line

            // Y-axis line (green)
            LineVertex{{0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // Start of green line
            LineVertex{{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // End of green line

            // Z-axis line (blue)
            LineVertex{{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, // Start of blue line
            LineVertex{{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}} // End of blue line
        }
    );
}

Entity EditorUtils::createRotationGizmo(World& world)
{
    return createGizmo
    (
        world,
        Primitives::generateCone(1, 1, 30)
    );
}

Entity EditorUtils::createScaleGizmo(World& world)
{
    return createGizmo
    (
        world,
        {
            // X-axis line (red)
            LineVertex{{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}}, // Start of red line
            LineVertex{{1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}}, // End of red line

            // Y-axis line (green)
            LineVertex{{0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 1.0f}}, // Start of green line
            LineVertex{{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 1.0f}}, // End of green line

            // Z-axis line (blue)
            LineVertex{{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 1.0f}}, // Start of blue line
            LineVertex{{0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f}} // End of blue line
        }
    );
}

constexpr std::vector<LineVertex> generateAABBVertices(const Vec3& min, const Vec3& max)
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

Entity EditorUtils::createBoundingBoxGizmo(World& world, Entity parentEntity)
{
    const BoundingBoxComponent& aabb = world.readComponent<BoundingBoxComponent>(parentEntity);
    const TransformComponent& transform = world.readComponent<TransformComponent>(parentEntity);
    std::string_view name = NameUtils::getName(world, parentEntity);

    Entity aabbGizmo = world.createEntity();
    world.addComponent<NameComponent>(aabbGizmo, std::format("BoundingBoxGizmo_{}", name));
    world.addComponent<TagsComponent>(aabbGizmo, {{Tag::notEditable}});
    world.addComponent<TransformComponent>(aabbGizmo, transform);
    world.addComponent<LineRenderComponent>(aabbGizmo, {.parent = parentEntity, .vertices = generateAABBVertices(aabb.minLocal, aabb.maxLocal)});

    return aabbGizmo;
}
