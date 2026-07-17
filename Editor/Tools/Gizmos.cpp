module Editor.Gizmos;
import AssetManager;
import Assets.Mesh;
import Components.BoundingBox;
import Components.EntityProxy;
import Components.Gizmo;
import Components.Hierarchy;
import Components.LineRender;
import Components.Model;
import Components.Name;
import Components.Render;
import Components.Tags;
import Components.Transform;
import Engine;
import Geometry;
import Guid;
import Math;
import Physics;
import Render.Commands;
import Render.Primitives;
import Systems.Transform;

namespace Gizmos
{
    Entity createTranslationGizmo(World& world);
    Entity createTranslationGizmoHandle(World& world, Entity gizmo, GizmoHandleType type, std::string name, std::vector<LineVertex>&& vertices, BoundingBoxComponent boundingBox);
    Entity createTranslationGizmoHandle(World& world, Entity gizmo, GizmoHandleType type, std::string name, const Guid& mesh, BoundingBoxComponent boundingBox);

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
    world.addComponent<TagsComponent>(gizmo, {{Tag::editorOnly}});
    world.addComponent<HierarchyComponent>(gizmo);
    world.addComponent<TransformComponent>(gizmo, {.position = Vec3{}, .rotation = Quat{}, .scale = 0.2});

    static constexpr float hitSize = 0.1f;

    static constexpr Vec4 xColor = {1.0f, 0.0f, 0.0f, 0.5f};
    const Entity x = createTranslationGizmoHandle
    (
        world,
        gizmo,
        GizmoHandleType::TranslateX,
        "X",
        {
            LineVertex{{0.0f, 0.0f, 0.0f}, xColor},
            LineVertex{{1.0f, 0.0f, 0.0f}, xColor}
        },
        {
            .channel = {},
            .minLocal = {-hitSize, -hitSize, -hitSize},
            .maxLocal = {1 + hitSize, hitSize, hitSize}
        }
    );

    static constexpr Vec4 yColor = {0.0f, 1.0f, 0.0f, 0.5f};
    const Entity y = createTranslationGizmoHandle
    (
        world,
        gizmo,
        GizmoHandleType::TranslateY,
        "Y",
        {
            LineVertex{{0.0f, 0.0f, 0.0f}, yColor},
            LineVertex{{0.0f, 1.0f, 0.0f}, yColor}
        },
        {
            .channel = {},
            .minLocal = {-hitSize, -hitSize, -hitSize},
            .maxLocal = {hitSize, 1 + hitSize, hitSize}
        }
    );
    
    static constexpr Vec4 zColor = {0.0f, 0.0f, 1.0f, 0.5f};
    const Entity z = createTranslationGizmoHandle
    (
        world,
        gizmo,
        GizmoHandleType::TranslateZ,
        "Z",
        {
            LineVertex{{0.0f, 0.0f, 0.0f}, zColor},
            LineVertex{{0.0f, 0.0f, 1.0f}, zColor}
        },
        {
            .channel = {},
            .minLocal = {-hitSize, -hitSize, -hitSize},
            .maxLocal = {hitSize, hitSize, 1 + hitSize}
        }
    );

    static constexpr float min = 0.1f;
    static constexpr float max = 0.5f;

    static constexpr Vec4 xyColor = zColor;
    const Guid xyMesh = AssetManager::add("TranslateXY", MeshData{
        .vertices = {
            {.pos = {min, min, 0.0f}, .color = xyColor},
            {.pos = {max, min, 0.0f}, .color = xyColor},
            {.pos = {max, max, 0.0f}, .color = xyColor},
            {.pos = {min, max, 0.0f}, .color = xyColor},
            {.pos = {min, max, 0.0f}, .color = xyColor},
        },
        .indices = {0, 1, 2, 2, 3, 0}
    });

    const Entity xy = createTranslationGizmoHandle
    (
        world,
        gizmo,
        GizmoHandleType::TranslateXY,
        "XY",
        xyMesh,
        {
            .channel = {},
            .minLocal = {min, min, -hitSize},
            .maxLocal = {max, max, hitSize}
        }
    );

    static constexpr Vec4 xzColor = yColor;
    const Guid xzMesh = AssetManager::add("TranslateXZ", MeshData{
        .vertices = {
            {.pos = {min, 0.f, min}, .color = xzColor},
            {.pos = {max, 0.f, min}, .color = xzColor},
            {.pos = {max, 0.f, max}, .color = xzColor},
            {.pos = {min, 0.f, max}, .color = xzColor},
            {.pos = {min, 0.f, max}, .color = xzColor},
        },
        .indices = {0, 1, 2, 2, 3, 0}
    });

    const Entity xz = createTranslationGizmoHandle
    (
        world,
        gizmo,
        GizmoHandleType::TranslateXZ,
        "XZ",
        xzMesh,
        {
            .channel = {},
            .minLocal = {min, -hitSize, min},
            .maxLocal = {max, hitSize, max}
        }
    );

    static constexpr Vec4 yzColor = xColor;
    const Guid yzMesh = AssetManager::add("TranslateYZ", MeshData{
        .vertices = {
            {.pos = {0.f, min, min}, .color = yzColor},
            {.pos = {0.f, max, min}, .color = yzColor},
            {.pos = {0.f, max, max}, .color = yzColor},
            {.pos = {0.f, min, max}, .color = yzColor},
            {.pos = {0.f, min, max}, .color = yzColor},
        },
        .indices = {0, 1, 2, 2, 3, 0}
    });

    const Entity yz = createTranslationGizmoHandle
    (
        world,
        gizmo,
        GizmoHandleType::TranslateYZ,
        "YZ",
        yzMesh,
        {
            .channel = {},
            .minLocal = {-hitSize, min, min},
            .maxLocal = {hitSize, max, max}
        }
    );
    
    world.addComponent<GizmoComponent>(gizmo, {.xAxis = x, .yAxis = y, .zAxis = z, .xyPlane = xy, .xzPlane = xz, .yzPlane = yz});
    setGizmoVisible(world, gizmo, false);
    log(std::format("Translation Gizmo: {} ({}, {}, {})", gizmo, x, y, z));
    
    return gizmo;
}

Entity createTranslationGizmoHandle(World& world, Entity gizmo, GizmoHandleType type, std::string name, BoundingBoxComponent boundingBox)
{
    const Entity axis = world.createEntity();
    world.addComponent<NameComponent>(axis, std::move(name));
    world.addComponent<TagsComponent>(axis, {{Tag::editorOnly}});
    world.addComponent<GizmoHandleComponent>(axis, {type});
    world.addComponent<HierarchyComponent>(axis);
    HierarchyUtils::setParent(world, axis, gizmo);
    world.addComponent<TransformComponent>(axis);
    world.addComponent<BoundingBoxComponent>(axis, std::move(boundingBox));

    return axis;
}

Entity Gizmos::createTranslationGizmoHandle(World& world, Entity gizmo, GizmoHandleType type, std::string name, std::vector<LineVertex>&& vertices, BoundingBoxComponent boundingBox)
{
    const Entity axis = createTranslationGizmoHandle(world, gizmo, type, std::move(name), std::move(boundingBox));
    world.addComponent<LineRenderComponent>(axis, std::move(vertices));

    return axis;
}

Entity Gizmos::createTranslationGizmoHandle(World& world, Entity gizmo, GizmoHandleType type, std::string name, const Guid& mesh, BoundingBoxComponent boundingBox)
{
    const Entity axis = createTranslationGizmoHandle(world, gizmo, type, std::move(name), std::move(boundingBox));
    world.addComponent<ModelComponent>(axis, {.mesh = mesh, .layer = RenderLayer::Gizmo});

    return axis;
}

Entity Gizmos::createRotationGizmo(World& world)
{
    // TODO(feature): Implement rotation gizmo
    Entity gizmo = createTranslationGizmo(world);
    world.editComponent<NameComponent>(gizmo)->name = "Rotation Gizmo";
    return gizmo;
}

Entity Gizmos::createScaleGizmo(World& world)
{
    // TODO(feature): Implement scale gizmo
    Entity gizmo = createTranslationGizmo(world);
    world.editComponent<NameComponent>(gizmo)->name = "Scale Gizmo";
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

Entity Gizmos::createBoundingBoxGizmo(World& editorWorld, const World& sourceEntityWorld,  Entity sourceEntity)
{
    std::string_view name = NameUtils::getName(sourceEntityWorld, sourceEntity);

    Entity aabbGizmo = editorWorld.createEntity();

    editorWorld.addComponent<NameComponent>(aabbGizmo, std::format("BoundingBoxGizmo_{}", name));
    editorWorld.addComponent<TagsComponent>(aabbGizmo, {{Tag::editorOnly}});

    editorWorld.addComponent<RuntimeTransformComponent>(aabbGizmo);
    editorWorld.addComponent<EntityProxyComponent>(aabbGizmo, {.sourceWorld = sourceEntityWorld.getHandle(), .sourceEntity = sourceEntity});

    const BoundingBoxComponent& aabb = sourceEntityWorld.readComponent<BoundingBoxComponent>(sourceEntity);
    editorWorld.addComponent<LineRenderComponent>(aabbGizmo, {.vertices = generateAABBVertices(aabb.minLocal, aabb.maxLocal)});

    return aabbGizmo;
}

void Gizmos::setGizmoVisible(World& world, Entity gizmo, bool visible)
{
    auto setVisible = [&](Entity entity)
    {
        if (!entity.isValid()) return;
        if (visible)
        {
            TransformSystem::ensureRuntimeTransform(world, gizmo);
            Engine::getRenderCommandQueue().addCommand(RenderCommands::SetTransform{world.getHandle(), entity, world.readComponent<RuntimeTransformComponent>(entity).worldMatrix});
        }
        Engine::getRenderCommandQueue().addCommand(RenderCommands::SetObjectVisibility{world.getHandle(), entity, visible});
    };

    setVisible(gizmo);

    if (world.hasComponent<GizmoComponent>(gizmo))
    {
        const auto& gizmoComponent = world.readComponent<GizmoComponent>(gizmo);
        GizmoUtils::forEachHandle(gizmoComponent, setVisible);
    }
}
