module Editor.TransformTool;
import :Detail;
import Engine.Camera;
import Engine;
import Input;
import Physics;
import World;
import Editor.Requests;
import Editor;

// Not using this for now
void capturePositions(const World& world, std::span<const Entity> entities, std::unordered_map<Entity, Vec3>& out)
{
    out.clear();
    for (Entity entity : entities)
        out[entity] = world.readComponent<TransformComponent>(entity).position;
}

void TranslateTool::update()
{
    TransformToolImpl::update();

    if (!context().viewportId)
        return;

    World& world = Engine::getWorld(context().editing.world);
    World& editorWorld = Engine::getWorld(context().editing.editorWorld);

    std::span<const Entity> selection = context().editing.selection.get();
    if (selection.empty())
        return;

    Entity firstSelectedEntity = selection.front();
    if (getSelectedHandle() != GizmoHandleType::None && world.isValid(firstSelectedEntity) && Input::isKeyDown(KeyCode::MouseButtonLeft))
    {
        const TransformComponent& transform = world.readComponent<TransformComponent>(firstSelectedEntity);

        auto projectCursor = [&](TranslationConstraint constraint)
        {
            Vec3 planeNormal;
            if (constraint.type == TranslationConstraint::Type::Axis)
            {
                const Camera& camera = context().services.viewports.getCamera(context().viewportId);
                planeNormal = Math::cross(CameraUtils::right(camera), constraint.direction);
            }
            else
            {
                planeNormal = constraint.direction;
            }

            const Plane movePlane
            {
                .point = transform.position,
                .normal = planeNormal
            };

            const Ray ray = Engine::getViewportCursorRay(context().viewportId);
            return Physics::intersectRayPlane(ray, movePlane);
        };

        auto getDelta = [&](TranslationConstraint constraint, Vec3 projectedCursorPosition)
        {
            if (constraint.type == TranslationConstraint::Type::Axis)
            {
                const float amount = Math::dot(projectedCursorPosition - *m_projectedCursorPositionLastFrame, constraint.direction);
                return constraint.direction * amount;
            }
            return projectedCursorPosition - *m_projectedCursorPositionLastFrame;
        };

        auto getConstraint = [&]() -> TranslationConstraint
        {
            switch (getSelectedHandle())
            {
                case GizmoHandleType::TranslateX:
                    return {TranslationConstraint::Type::Axis, TransformUtils::right(transform)};
                case GizmoHandleType::TranslateY:
                    return {TranslationConstraint::Type::Axis, TransformUtils::up(transform)};
                case GizmoHandleType::TranslateZ:
                    return {TranslationConstraint::Type::Axis, TransformUtils::forward(transform)};
                case GizmoHandleType::TranslateXY:
                    return {TranslationConstraint::Type::Plane, TransformUtils::forward(transform)};
                case GizmoHandleType::TranslateXZ:
                    return {TranslationConstraint::Type::Plane, TransformUtils::up(transform)};
                case GizmoHandleType::TranslateYZ:
                    return {TranslationConstraint::Type::Plane, TransformUtils::right(transform)};

                case GizmoHandleType::None:
                default:
                    report("Invalid gizmo axis entity");
                    return {};
            }
        };

        const TranslationConstraint constraint = getConstraint();
        const std::optional<Vec3> projectedCursorPosition = projectCursor(constraint);

        if (m_projectedCursorPositionLastFrame && projectedCursorPosition)
        {
            auto selectionTransform = world.editComponent<TransformComponent>(firstSelectedEntity);
            selectionTransform->position += getDelta(constraint, *projectedCursorPosition);
        }
        m_projectedCursorPositionLastFrame = projectedCursorPosition;
    }
    else
    {
        setSelectedHandle(GizmoHandleType::None);
        m_projectedCursorPositionLastFrame = {};
        setSelectionEnabled(true);
    }

    if (Input::isKeyJustPressed(KeyCode::MouseButtonLeft))
    {
        const Ray ray = Engine::getViewportCursorRay(context().viewportId);
        const Entity hitGizmoHandle = Physics::lineTrace(editorWorld, ray, TraceChannelFlags::Gizmo);
        setSelectedHandle(getHandleType(editorWorld, hitGizmoHandle));

        if (getSelectedHandle() != GizmoHandleType::None)
            setSelectionEnabled(false);

        m_projectedCursorPositionLastFrame = {};
    }
}
