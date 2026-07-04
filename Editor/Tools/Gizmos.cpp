module Editor.Gizmos;
import Assets.Mesh;
import Component.LineRender;
import Component.BoundingBox;
import Component.Gizmo;
import Component.Name;
import Component.Model;
import Component.Hierarchy;
import Component.Render;
import Component.Tags;
import Component.Transform;
import Guid;
import Math;
import Physics;
import Render.Commands;
import Render.Primitives;

namespace Gizmos
{
    Entity createTranslationGizmo(World& world);
    Entity createTranslationGizmoAxis(World& world, Entity gizmo, std::string name, std::vector<LineVertex>&& vertices, BoundingBoxComponent boundingBox);

    Entity createRotationGizmo(World& world);
    Entity createScaleGizmo(World& world);
}

Entity Gizmos::createTransformGizmo(World& world, EntityEditingMode type)
{
    switch (type)
    {
        case EntityEditingMode::Translate:
            return createTranslationGizmo(world);
        case EntityEditingMode::Rotate:
            return createRotationGizmo(world);
        case EntityEditingMode::Scale:
            return createScaleGizmo(world);
        case EntityEditingMode::None:
        default:
        {
            report("Tried to create invalid gizmo type");
            return {};
        }
    }
}

Entity Gizmos::createTranslationGizmo(World& world)
{
    const Entity gizmo = world.createEntity();
    world.addComponent<NameComponent>(gizmo, "Translation Gizmo");
    world.addComponent<HierarchyComponent>(gizmo);
    world.addComponent<TagsComponent>(gizmo, {{Tag::editorOnly}});
    world.addComponent<TransformComponent>(gizmo);

    static constexpr Vec3 xColor = {1.0f, 0.0f, 0.0f};
    const Entity x = createTranslationGizmoAxis
    (
        world,
        gizmo,
        "X",
        {
            LineVertex{{0.0f, 0.0f, 0.0f}, xColor},
            LineVertex{{1.0f, 0.0f, 0.0f}, xColor}
        },
        {
            .channel = {},
            .minLocal = {-0.2f, -0.2f, -0.2f},
            .maxLocal = {1.2f, 0.2f, 0.2f}
        }
    );

    static constexpr Vec3 yColor = {0.0f, 1.0f, 0.0f};
    const Entity y = createTranslationGizmoAxis
    (
        world,
        gizmo,
        "Y",
        {
            LineVertex{{0.0f, 0.0f, 0.0f}, yColor},
            LineVertex{{0.0f, 1.0f, 0.0f}, yColor}
        },
        {
            .channel = {},
            .minLocal = {-0.2f, -0.2f, -0.2f},
            .maxLocal = {0.2f, 1.2f, 0.2f}
        }
    );
    
    static constexpr Vec3 zColor = {0.0f, 0.0f, 1.0f};
    const Entity z = createTranslationGizmoAxis
    (
        world,
        gizmo,
        "Z",
        {
            LineVertex{{0.0f, 0.0f, 0.0f}, zColor},
            LineVertex{{0.0f, 0.0f, 1.0f}, zColor}
        },
        {
            .channel = {},
            .minLocal = {-0.2f, -0.2f, -0.2f},
            .maxLocal = {0.2f, 0.2f, 1.2f}
        }
    );
    
    world.addComponent<GizmoComponent>(gizmo, {.xAxisEntity = x, .yAxisEntity = y, .zAxisEntity = z});
    setGizmoVisible(world, gizmo, false);
    
    return gizmo;
}

Entity Gizmos::createTranslationGizmoAxis(World& world, Entity gizmo, std::string name, std::vector<LineVertex>&& vertices, BoundingBoxComponent boundingBox)
{
    const Entity axis = world.createEntity();
    world.addComponent<BoundingBoxComponent>(axis, std::move(boundingBox));
    world.addComponent<LineRenderComponent>(axis, std::move(vertices));
    world.addComponent<NameComponent>(axis, std::move(name));
    world.addComponent<HierarchyComponent>(axis);
    world.addComponent<TagsComponent>(axis, {{Tag::editorOnly}});
    world.addComponent<TransformComponent>(axis);
    HierarchyUtils::setParent(world, axis, gizmo);

    return axis;
}

Entity Gizmos::createRotationGizmo(World& world)
{
    // TODO(feature): Implement rotation gizmo
    Entity gizmo = createTranslationGizmo(world);
    world.editComponent<NameComponent>(gizmo).name = "Rotation Gizmo";
    return gizmo;
}

Entity Gizmos::createScaleGizmo(World& world)
{
    // TODO(feature): Implement scale gizmo
    Entity gizmo = createTranslationGizmo(world);
    world.editComponent<NameComponent>(gizmo).name = "Scale Gizmo";
    return gizmo;
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

Entity Gizmos::createBoundingBoxGizmo(World& world, Entity parentEntity)
{
    const BoundingBoxComponent& aabb = world.readComponent<BoundingBoxComponent>(parentEntity);
    std::string_view name = NameUtils::getName(world, parentEntity);

    Entity aabbGizmo = world.createEntity();
    world.addComponent<LineRenderComponent>(aabbGizmo, {.vertices = generateAABBVertices(aabb.minLocal, aabb.maxLocal)});
    world.addComponent<NameComponent>(aabbGizmo, std::format("BoundingBoxGizmo_{}", name));
    world.addComponent<HierarchyComponent>(aabbGizmo);
    HierarchyUtils::setParent(world, aabbGizmo, parentEntity);
    world.addComponent<TagsComponent>(aabbGizmo, {{Tag::editorOnly}});
    world.addComponent<TransformComponent>(aabbGizmo);

    return aabbGizmo;
}

void Gizmos::setGizmoVisible(World& world, Entity gizmo, bool visible)
{
    auto setVisible = [&](Entity entity)
    {
        if (!entity.isValid()) return;
        world.getRenderManager().addCommand(RenderCommands::SetObjectVisibility{entity, visible});
    };

    setVisible(gizmo);

    if (world.hasComponent<GizmoComponent>(gizmo))
    {
        auto& gizmoComponent = world.editComponent<GizmoComponent>(gizmo);
        setVisible(gizmoComponent.xAxisEntity);
        setVisible(gizmoComponent.yAxisEntity);
        setVisible(gizmoComponent.zAxisEntity);
    }
}
