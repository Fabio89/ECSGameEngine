module Editor.Gizmos;
import Component.LineRender;
import Component.BoundingBox;
import Component.Gizmo;
import Component.Name;
import Component.Model;
import Component.Parent;
import Component.Render;
import Component.Tags;
import Component.Transform;
import Guid;
import Math;
import Physics;
import Render.Model;
import Render.Primitives;

Entity createTranslationGizmoAxis(World& world, Entity gizmo, std::string name, std::vector<LineVertex>&& vertices, BoundingBoxComponent boundingBox)
{
    const Entity axis = world.createEntity();
    world.addComponent<BoundingBoxComponent>(axis, std::move(boundingBox));
    world.addComponent<LineRenderComponent>(axis, std::move(vertices));
    world.addComponent<NameComponent>(axis, std::move(name));
    world.addComponent<ParentComponent>(axis, gizmo);
    world.addComponent<TagsComponent>(axis, {{Tag::notEditable}});
    world.addComponent<TransformComponent>(axis);
    return axis;
}

Entity EditorUtils::createTranslationGizmo(World& world)
{
    const Entity gizmo = world.createEntity();
    world.addComponent<NameComponent>(gizmo, "Translation Gizmo");
    world.addComponent<ParentComponent>(gizmo);
    world.addComponent<TagsComponent>(gizmo, {{Tag::notEditable}});
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
            .channel = TraceChannel{TraceChannelFlags::Gizmo},
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
            .channel = TraceChannel{TraceChannelFlags::Gizmo},
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
            .channel = TraceChannel{TraceChannelFlags::Gizmo},
            .minLocal = {-0.2f, -0.2f, -0.2f},
            .maxLocal = {0.2f, 0.2f, 1.2f}
        }
    );
    
    world.addComponent<GizmoComponent>(gizmo, {.xAxisEntity = x, .yAxisEntity = y, .zAxisEntity = z});
    setGizmoVisible(world, gizmo, false);
    
    return gizmo;
}

Entity EditorUtils::createRotationGizmo(World& world)
{
    return createTranslationGizmo(world);
    // return createGizmo
    // (
    //     world,
    //     Primitives::generateCone(1, 1, 30)
    // );
}

Entity EditorUtils::createScaleGizmo(World& world)
{
    return createTranslationGizmo(world);
}

void EditorUtils::setGizmoVisible(World& world, Entity gizmoEntity, bool visible)
{
    world.getRenderManager().addCommand(RenderCommands::SetObjectVisibility{gizmoEntity, visible});

    if (world.hasComponent<GizmoComponent>(gizmoEntity))
    {
        const GizmoComponent& gizmoComponent = world.readComponent<GizmoComponent>(gizmoEntity);
        world.getRenderManager().addCommand(RenderCommands::SetObjectVisibility{gizmoComponent.xAxisEntity, visible});
        world.getRenderManager().addCommand(RenderCommands::SetObjectVisibility{gizmoComponent.yAxisEntity, visible});
        world.getRenderManager().addCommand(RenderCommands::SetObjectVisibility{gizmoComponent.zAxisEntity, visible});
    }
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
    std::string_view name = NameUtils::getName(world, parentEntity);

    Entity aabbGizmo = world.createEntity();
    world.addComponent<LineRenderComponent>(aabbGizmo, {.vertices = generateAABBVertices(aabb.minLocal, aabb.maxLocal)});
    world.addComponent<NameComponent>(aabbGizmo, std::format("BoundingBoxGizmo_{}", name));
    world.addComponent<ParentComponent>(aabbGizmo, parentEntity);
    world.addComponent<TagsComponent>(aabbGizmo, {{Tag::notEditable}});
    world.addComponent<TransformComponent>(aabbGizmo);

    return aabbGizmo;
}
