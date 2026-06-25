module Editor;
import Component.Hierarchy;
import Component.Transform;
import Editor.Camera;
import Editor.Components;
import Editor.Gizmos;
import Engine;
import Input;
import Math;
import Physics;
import UI.Widget.EntityExplorer;
import UI.Widget.MainMenu;
import World;

enum class EditMode : UInt8
{
    Translate = 0,
    Rotate = 1,
    Scale = 2,
    None = 3
};

namespace
{
    EditMode currentEditMode{EditMode::None};
    std::array<Entity, 3> gizmos;
    std::unordered_map<Entity, Entity> entitiesToBoundingBoxGizmos;
    Entity selected = invalidId();
    Entity selectedGizmoAxis = invalidId();
    std::optional<Vec3> projectedCursorPositionLastFrame;
}

constexpr Entity& getGizmo(EditMode editMode)
{
    return gizmos[static_cast<UInt8>(editMode)];
}

void attachToSelectedEntity(World& world, Entity gizmo)
{
    HierarchyUtils::setParent(world, gizmo, selected);
    world.editComponent<TransformComponent>(gizmo).scale = 0.2f / world.readComponent<TransformComponent>(selected).scale;
}

void setEditMode(World& world, EditMode editMode)
{
    if (editMode != currentEditMode)
    {
        currentEditMode = editMode;
        for (Entity gizmo : gizmos)
        {
            check(world.isValid(gizmo), "Gizmo entity is invalid. Please ensure all gizmos are properly initialized before setting edit mode.", ErrorType::Error);
            const bool shouldShow = Engine::isValid(selected) && editMode != EditMode::None && gizmo == getGizmo(editMode);
            EditorUtils::setGizmoVisible(world, gizmo, shouldShow);
            if (shouldShow)
            {
                attachToSelectedEntity(world, gizmo);
            }
        }
    }
}

void Editor::init(World& world)
{
    EditorComponents::init();

    world.addWidget<Widgets::EntityExplorer>();
    world.addWidget<Widgets::MainMenuWidget>();
}

void Editor::createGizmos(World& world)
{
    getGizmo(EditMode::Translate) = EditorUtils::createTranslationGizmo(world);
    getGizmo(EditMode::Rotate) = EditorUtils::createRotationGizmo(world);
    getGizmo(EditMode::Scale) = EditorUtils::createScaleGizmo(world);
}

void Editor::update(World& world, WindowHandle window, float deltaTime)
{
    if (Input::isKeyJustPressed(KeyCode::Q))
    {
        setEditMode(world, EditMode::None);
    } else if (Input::isKeyJustPressed(KeyCode::W))
    {
        setEditMode(world, EditMode::Translate);
    } else if (Input::isKeyJustPressed(KeyCode::E))
    {
        setEditMode(world, EditMode::Rotate);
    } else if (Input::isKeyJustPressed(KeyCode::R))
    {
        setEditMode(world, EditMode::Scale);
    }

    const Vec2 cursorPosition = Input::getCursorPosition(window);
    const Ray ray = Physics::rayFromScreenPosition(world, Engine::getPlayer(), cursorPosition);

    if (Engine::isValid(selected) && Engine::isValid(selectedGizmoAxis) && Input::isKeyDown(KeyCode::MouseButtonLeft))
    {
        TransformComponent &transform = world.editComponent<TransformComponent>(selected);

        const Vec3 gizmoAxisDirection = [&]
        {
            const Entity gizmoEntity = HierarchyUtils::getParent(world, selectedGizmoAxis);
            const GizmoComponent &gizmo = world.readComponent<GizmoComponent>(gizmoEntity);
            if (selectedGizmoAxis == gizmo.xAxisEntity)
                return TransformUtils::right(transform);
            if (selectedGizmoAxis == gizmo.yAxisEntity)
                return TransformUtils::up(transform);
            if (selectedGizmoAxis == gizmo.zAxisEntity)
                return TransformUtils::forward(transform);
            report("Invalid gizmo axis entity");
            return Vec3{};
        }();

        const Plane movePlane
        {
            .point = transform.position,
            .normal = Math::cross(
                TransformUtils::right(world.readComponent<TransformComponent>(Engine::getPlayer().getMainCamera())),
                gizmoAxisDirection)
        };

        const std::optional<Vec3> projectedCursorPosition = Physics::intersectRayPlane(ray, movePlane);

        if (projectedCursorPositionLastFrame.has_value() && projectedCursorPosition.has_value())
        {
            const auto delta = Math::dot(*projectedCursorPosition - *projectedCursorPositionLastFrame,
                                         gizmoAxisDirection);
            transform.position += gizmoAxisDirection * delta;
            projectedCursorPositionLastFrame = *projectedCursorPosition;
        }
        projectedCursorPositionLastFrame = projectedCursorPosition;
    } else
    {
        selectedGizmoAxis = invalidId();
        projectedCursorPositionLastFrame = {};
    }

    if (Input::isKeyJustPressed(KeyCode::MouseButtonLeft))
    {
        selectedGizmoAxis = Physics::lineTrace(world, ray, TraceChannelFlags::Gizmo);

        const Entity previouslySelected = selected;
        selected = Physics::lineTrace(world, ray, TraceChannelFlags::Default);

        if (selected != previouslySelected)
        {
            if (previouslySelected != invalidId())
            {
                EditorUtils::setGizmoVisible(world, entitiesToBoundingBoxGizmos.at(previouslySelected), false);
            }

            if (selected != invalidId())
            {
                if (currentEditMode != EditMode::None)
                {
                    const Entity gizmo = getGizmo(currentEditMode);
                    attachToSelectedEntity(world, gizmo);
                    EditorUtils::setGizmoVisible(world, gizmo, true);
                }

                auto it = entitiesToBoundingBoxGizmos.find(selected);
                if (it == entitiesToBoundingBoxGizmos.end())
                    it = entitiesToBoundingBoxGizmos.emplace(
                        selected, EditorUtils::createBoundingBoxGizmo(world, selected)).first;

                const Entity boundingBoxGizmo = it->second;
                EditorUtils::setGizmoVisible(world, boundingBoxGizmo, true);
            } else
            {
                if (currentEditMode != EditMode::None)
                {
                    EditorUtils::setGizmoVisible(world, getGizmo(currentEditMode), false);
                }
            }
        }
    }

    EditorCamera::setActive(window, Input::isKeyDown(KeyCode::MouseButtonRight));
    EditorCamera::update(window, world, Engine::getPlayer(), deltaTime);
}
