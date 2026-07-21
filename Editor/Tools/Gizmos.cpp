module Editor.Gizmos;
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
    GizmoHandle createGizmoHandle(World& world, Entity gizmo, GizmoHandleType type);

    std::vector<GizmoHandle> createHandles(World& world, Entity gizmo, EntityEditingMode type);
}

struct GizmoResources
{
    Guid translateAxisMesh;
    Guid translatePlaneMesh;
    Guid scaleAxisMesh;
    Guid scaleUniformMesh;
    Guid rotateAxisMesh;
};

struct AngleAxis
{
    float degrees{};
    Vec3 axis{};
};

struct GizmoHandleConfig
{
    Vec4 color;
    Guid mesh;
    BoundingBoxComponent boundingBox;
    AngleAxis rotation;
};

namespace
{
    GizmoResources resources;

    constexpr float alpha = 0.75;
    constexpr Vec4 xColor = {1.0f, 0.0f, 0.0f, alpha};
    constexpr Vec4 yColor = {0.0f, 1.0f, 0.0f, alpha};
    constexpr Vec4 zColor = {0.0f, 0.0f, 1.0f, alpha};

    constexpr float hitSize = 0.1f;

    constexpr BoundingBoxComponent axisBoundingBox
    {
        .channel = {},
        .minLocal = {-hitSize, -hitSize, -hitSize},
        .maxLocal = {1 + hitSize, hitSize, hitSize}
    };

    constexpr float planeMin = 0.1f;
    constexpr float planeMax = 0.5f;

    constexpr BoundingBoxComponent planeBoundingBox
    {
        .channel = {},
        .minLocal = {planeMin, planeMin, -hitSize},
        .maxLocal = {planeMax, planeMax, hitSize}
    };

    std::unordered_map<GizmoHandleType, GizmoHandleConfig> gizmoConfig;
}

void Gizmos::init(AssetManager& assets, AssetMountId mount)
{
    resources.translateAxisMesh = assets.addFromFile<MeshData>("TranslateAxis", {.mountId = mount, .relativeToMount = "Meshes/TranslateAxis.obj"});
    resources.scaleAxisMesh = assets.addFromFile<MeshData>("ScaleAxis", {.mountId = mount, .relativeToMount = "Meshes/ScaleAxis.obj"});
    resources.scaleUniformMesh = assets.addFromFile<MeshData>("ScaleUniform", {.mountId = mount, .relativeToMount = "Meshes/ScaleUniform.obj"});
    resources.rotateAxisMesh = assets.addFromFile<MeshData>("RotateAxis", {.mountId = mount, .relativeToMount = "Meshes/RotateAxis2.obj"});

    BoundingBoxComponent scaleUniformBoundingBox = BoundingBoxUtils::computeBoundingBox(assets.resolve<MeshData>(resources.scaleUniformMesh));
    scaleUniformBoundingBox.minLocal -= Vec3{hitSize, hitSize, hitSize};
    scaleUniformBoundingBox.maxLocal += Vec3{hitSize, hitSize, hitSize};

    static constexpr float min = 0.2f;
    static constexpr float max = 0.5f;

    resources.translatePlaneMesh = assets.addFromData("TransformPlane", MeshData{
        .vertices = {
            {.pos = {min, min, 0.0f}},
            {.pos = {max, min, 0.0f}},
            {.pos = {max, max, 0.0f}},
            {.pos = {min, max, 0.0f}},
            {.pos = {min, max, 0.0f}},
        },
        .indices = {0, 1, 2, 2, 3, 0}
    });

    constexpr AngleAxis yRotation{90.f, forwardVector()};
    constexpr AngleAxis zRotation{-90.f, upVector()};
    constexpr AngleAxis xzRotation{90.f, rightVector()};
    constexpr AngleAxis yzRotation{-90.f, upVector()};

    gizmoConfig =
    {
        {GizmoHandleType::TranslateX, {xColor, resources.translateAxisMesh, axisBoundingBox}},
        {GizmoHandleType::TranslateY, {yColor, resources.translateAxisMesh, axisBoundingBox, yRotation}},
        {GizmoHandleType::TranslateZ, {zColor, resources.translateAxisMesh, axisBoundingBox, zRotation}},
        {GizmoHandleType::TranslateXY, {zColor, resources.translatePlaneMesh, planeBoundingBox}},
        {GizmoHandleType::TranslateXZ, {yColor, resources.translatePlaneMesh, planeBoundingBox, xzRotation}},
        {GizmoHandleType::TranslateYZ, {xColor, resources.translatePlaneMesh, planeBoundingBox, yzRotation}},

        {GizmoHandleType::ScaleX, {xColor, resources.scaleAxisMesh, axisBoundingBox}},
        {GizmoHandleType::ScaleY, {yColor, resources.scaleAxisMesh, axisBoundingBox, yRotation}},
        {GizmoHandleType::ScaleZ, {zColor, resources.scaleAxisMesh, axisBoundingBox, zRotation}},
        {GizmoHandleType::ScaleXY, {zColor, resources.translatePlaneMesh, planeBoundingBox}},
        {GizmoHandleType::ScaleXZ, {yColor, resources.translatePlaneMesh, planeBoundingBox, xzRotation}},
        {GizmoHandleType::ScaleYZ, {xColor, resources.translatePlaneMesh, planeBoundingBox, yzRotation}},
        {GizmoHandleType::ScaleUniform, {{1,1,1,alpha}, resources.scaleUniformMesh, scaleUniformBoundingBox}},

        {GizmoHandleType::RotateX, {xColor, resources.rotateAxisMesh, axisBoundingBox}},
        {GizmoHandleType::RotateY, {yColor, resources.rotateAxisMesh, axisBoundingBox, {-90.f, forwardVector()}}},
        {GizmoHandleType::RotateZ, {zColor, resources.rotateAxisMesh, axisBoundingBox, {90.f, upVector()}}},
    };
}

Entity Gizmos::createTransformGizmo(World& editorWorld, WorldHandle mainWorld, EntityEditingMode type)
{
    const Entity gizmo = editorWorld.createEntity();
    editorWorld.addComponent<NameComponent>(gizmo, "Gizmo");
    editorWorld.addComponent<TagsComponent>(gizmo, {{Tag::editorOnly}});
    editorWorld.addComponent<HierarchyComponent>(gizmo);
    editorWorld.addComponent<TransformComponent>(gizmo, {.scale = Vec3{0.2}});
    editorWorld.addComponent<GizmoComponent>(gizmo, createHandles(editorWorld, gizmo, type));
    editorWorld.addComponent<EntityProxyComponent>(gizmo, EntityProxyComponent{.sourceWorld = mainWorld, .flags = EntityProxyFlags::CopyPosition | EntityProxyFlags::CopyRotation});

    setGizmoVisible(editorWorld, gizmo, false);
    return gizmo;
}

GizmoHandle Gizmos::createGizmoHandle(World& world, Entity gizmo, GizmoHandleType type)
{
    auto it = gizmoConfig.find(type);
    if (it == gizmoConfig.end())
        return {};

    const GizmoHandleConfig& config = it->second;

    const Entity handle = world.createEntity();
    world.addComponent<NameComponent>(handle, std::format("GizmoHandle_{}", handle.value));
    world.addComponent<TagsComponent>(handle, {{Tag::editorOnly}});
    world.addComponent<GizmoHandleComponent>(handle, {type});
    world.addComponent<HierarchyComponent>(handle);
    HierarchyUtils::setParent(world, handle, gizmo);

    const Quat rotation = config.rotation.degrees != 0.f ? Math::angleAxis(Math::radians(config.rotation.degrees), config.rotation.axis) : Quat{};

    world.addComponent<TransformComponent>(handle, {.rotation = rotation});
    world.addComponent<BoundingBoxComponent>(handle, config.boundingBox);
    world.addComponent<ModelComponent>(handle, {.mesh = config.mesh, .layer = RenderLayer::Gizmo, .tint = config.color});

    return {.entity = handle, .type = type};
}

std::vector<GizmoHandle> Gizmos::createHandles(World& world, Entity gizmo, EntityEditingMode type)
{
    switch (type)
    {
        case EntityEditingMode::Translate:
            return {
                createGizmoHandle(world, gizmo, GizmoHandleType::TranslateX),
                createGizmoHandle(world, gizmo, GizmoHandleType::TranslateY),
                createGizmoHandle(world, gizmo, GizmoHandleType::TranslateZ),
                createGizmoHandle(world, gizmo, GizmoHandleType::TranslateXY),
                createGizmoHandle(world, gizmo, GizmoHandleType::TranslateXZ),
                createGizmoHandle(world, gizmo, GizmoHandleType::TranslateYZ)
            };
        case EntityEditingMode::Rotate:
            return {
                createGizmoHandle(world, gizmo, GizmoHandleType::RotateX),
                createGizmoHandle(world, gizmo, GizmoHandleType::RotateY),
                createGizmoHandle(world, gizmo, GizmoHandleType::RotateZ)
            };
        case EntityEditingMode::Scale:
            return {
                createGizmoHandle(world, gizmo, GizmoHandleType::ScaleX),
                createGizmoHandle(world, gizmo, GizmoHandleType::ScaleY),
                createGizmoHandle(world, gizmo, GizmoHandleType::ScaleZ),
                // createGizmoHandle(world, gizmo, GizmoHandleType::ScaleXY),
                // createGizmoHandle(world, gizmo, GizmoHandleType::ScaleXZ),
                // createGizmoHandle(world, gizmo, GizmoHandleType::ScaleYZ),
                createGizmoHandle(world, gizmo, GizmoHandleType::ScaleUniform)
            };
        case EntityEditingMode::None:
        default:
            return {};
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

    static constexpr Vec4 color{1.0f, 0.75f, 0.0f, 0.5f};
    std::vector<LineVertex> lineVertices;
    std::ranges::transform(vertices, std::back_inserter(lineVertices), [](const Vec3& pos)
    {
        return LineVertex{pos, color};
    });
    return lineVertices;
}

Entity Gizmos::createBoundingBoxGizmo(World& editorWorld, const World& sourceEntityWorld, Entity sourceEntity)
{
    std::string_view name = NameUtils::getName(sourceEntityWorld, sourceEntity);

    Entity aabbGizmo = editorWorld.createEntity();

    editorWorld.addComponent<NameComponent>(aabbGizmo, std::format("BoundingBoxGizmo_{}", name));
    editorWorld.addComponent<TagsComponent>(aabbGizmo, {{Tag::editorOnly}});

    editorWorld.addComponent<TransformComponent>(aabbGizmo);
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
