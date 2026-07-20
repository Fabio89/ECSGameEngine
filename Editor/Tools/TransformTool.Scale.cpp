module Editor.TransformTool;
import :Detail;
import Engine.Camera;
import Engine;
import Input;
import Physics;
import World;
import Editor.Requests;
import Editor;

void ScaleTool::update()
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
        if (getSelectedHandle() == GizmoHandleType::ScaleUniform)
        {
            Vec2 currentMousePosition = Platform::Window::getCursorPosition(context().window);
            static constexpr float uniformScaleMultiplier = 0.005f;

            const Vec2 delta = m_previousMousePosition ? currentMousePosition - *m_previousMousePosition : Vec2{};

            auto selectionTransform = world.editComponent<TransformComponent>(firstSelectedEntity);
            selectionTransform->scale += Vec3{delta.x} * uniformScaleMultiplier;

            m_previousMousePosition = currentMousePosition;
        }
        else
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
                    case GizmoHandleType::ScaleX:
                        return {TranslationConstraint::Type::Axis, TransformUtils::right(transform)};
                    case GizmoHandleType::ScaleY:
                        return {TranslationConstraint::Type::Axis, TransformUtils::up(transform)};
                    case GizmoHandleType::ScaleZ:
                        return {TranslationConstraint::Type::Axis, TransformUtils::forward(transform)};
                    case GizmoHandleType::ScaleXY:
                        return {TranslationConstraint::Type::Plane, TransformUtils::forward(transform)};
                    case GizmoHandleType::ScaleXZ:
                        return {TranslationConstraint::Type::Plane, TransformUtils::up(transform)};
                    case GizmoHandleType::ScaleYZ:
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
                const Vec3 worldDelta = getDelta(constraint, *projectedCursorPosition);;

                selectionTransform->scale.x += Math::dot(worldDelta, TransformUtils::right(transform));
                selectionTransform->scale.y += Math::dot(worldDelta, TransformUtils::up(transform));
                selectionTransform->scale.z += Math::dot(worldDelta, TransformUtils::forward(transform));
            }
            m_projectedCursorPositionLastFrame = projectedCursorPosition;
        }
    }
    else
    {
        setSelectedHandle(GizmoHandleType::None);
        m_projectedCursorPositionLastFrame = {};
        m_previousMousePosition = {};
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
