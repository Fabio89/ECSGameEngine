module Editor.TransformTool;
import Components.Gizmo;
import Editor;
import Editor.Events;
import Editor.Gizmos;
import Editor.Selection;
import Engine;
import Engine.Camera;
import Input;
import Math;
import Physics;
import World;

TransformToolManager::TransformToolManager(TransformToolContext context)
    : EditorServiceConsumer{context.services},
      m_context{std::move(context)}
{
    m_sub += m_context.services.events.subscribe([this](const EditorEvents::SelectionChanged& event)
    {
        if (event.contextId == m_context.editing.id && m_currentToolType != EntityEditingMode::None)
        {
            TransformTool& currentTool = *m_tools[static_cast<std::size_t>(m_currentToolType)];
            currentTool.setActive(!event.selection.empty());
        }
    });

    createTools();
}

void TransformToolManager::createTools()
{
    m_tools[static_cast<int>(EntityEditingMode::Translate)] = std::make_unique<TranslateTool>(m_context);
    m_tools[static_cast<int>(EntityEditingMode::Rotate)] = std::make_unique<RotateTool>(m_context);
    m_tools[static_cast<int>(EntityEditingMode::Scale)] = std::make_unique<ScaleTool>(m_context);
}

void TransformToolManager::setCurrentTool(EntityEditingMode type)
{
    if (m_currentToolType == type)
        return;

    if (m_currentToolType != EntityEditingMode::None)
    {
        TransformTool& previousTool = *m_tools[static_cast<std::size_t>(m_currentToolType)];
        previousTool.setActive(false);
    }

    if (type != EntityEditingMode::None)
    {
        TransformTool& newTool = *m_tools[static_cast<std::size_t>(type)];
        newTool.setActive(true);
    }

    m_currentToolType = type;
}

void TransformToolManager::update()
{
    if (m_currentToolType != EntityEditingMode::None)
    {
        TransformTool& currentTool = *m_tools[static_cast<std::size_t>(m_currentToolType)];
        currentTool.update();
    }
}

TransformTool::TransformTool(TransformToolContext& context, EntityEditingMode type)
    : EditorServiceConsumer{context.services},
      m_context{context},
      m_gizmo{Gizmos::createTransformGizmo(Engine::getWorld(context.editing.editorWorld), type)} {}

void TransformTool::update()
{
    attachToSelection();
}

void TransformTool::setActive(bool active)
{
    World& editorWorld = Engine::getWorld(m_context.editing.editorWorld);
    if (!active)
        Gizmos::setGizmoVisible(editorWorld, m_gizmo, false);

    if (editorWorld.hasComponent<GizmoComponent>(m_gizmo))
    {
        auto setAxis = [active, &editorWorld](Entity axis)
        {
            if (!axis.isValid()) return;
            auto collision = editorWorld.editComponent<BoundingBoxComponent>(axis);
            collision->channel.set(TraceChannelFlags::Gizmo, active);
        };

        auto& gizmoComponent = editorWorld.readComponent<GizmoComponent>(m_gizmo);

        setAxis(gizmoComponent.xAxisEntity);
        setAxis(gizmoComponent.yAxisEntity);
        setAxis(gizmoComponent.zAxisEntity);
    }

    if (active)
    {
        attachToSelection();
        Gizmos::setGizmoVisible(editorWorld, m_gizmo, !context().editing.selection.isEmpty());
    }
}

void TransformTool::attachToSelection()
{
    Entity firstSelected = context().editing.selection.isEmpty() ? Entity{} : context().editing.selection.get().front();
    if (firstSelected.isValid())
    {
        const World& mainWorld = Engine::getWorld(m_context.editing.world);
        World& editorWorld = Engine::getWorld(m_context.editing.editorWorld);

        auto gizmoTransform = editorWorld.editComponent<TransformComponent>(m_gizmo);
        const auto& selectionTransform = mainWorld.readComponent<TransformComponent>(firstSelected);

        gizmoTransform->position = selectionTransform.position;
        gizmoTransform->rotation = selectionTransform.rotation;

    }
    m_attachedTo = firstSelected;
}

void TranslateTool::update()
{
    TransformTool::update();

    World& world = Engine::getWorld(context().editing.world);
    World& editorWorld = Engine::getWorld(context().editing.editorWorld);

    if (context().editing.selection.isEmpty())
        return;

    Entity firstSelectedEntity = context().editing.selection.get().front();
    if (world.isValid(firstSelectedEntity) && editorWorld.isValid(m_selectedGizmoAxis) && Input::isKeyDown(KeyCode::MouseButtonLeft))
    {
        const TransformComponent& transform = world.readComponent<TransformComponent>(firstSelectedEntity);

        const Vec3 gizmoAxisDirection = [&]
        {
            const Entity gizmoEntity = HierarchyUtils::getParent(editorWorld, m_selectedGizmoAxis);
            const GizmoComponent& gizmo = editorWorld.readComponent<GizmoComponent>(gizmoEntity);
            if (m_selectedGizmoAxis == gizmo.xAxisEntity)
                return TransformUtils::right(transform);
            if (m_selectedGizmoAxis == gizmo.yAxisEntity)
                return TransformUtils::up(transform);
            if (m_selectedGizmoAxis == gizmo.zAxisEntity)
                return TransformUtils::forward(transform);
            report("Invalid gizmo axis entity");
            return Vec3{};
        }();

        const Camera& camera = context().services.viewports.getCamera(context().viewportId);
        const Plane movePlane
        {
            .point = transform.position,
            .normal = Math::cross(CameraUtils::right(camera), gizmoAxisDirection)
        };

        const Ray ray = Engine::getViewportCursorRay(context().viewportId);
        const std::optional<Vec3> projectedCursorPosition = Physics::intersectRayPlane(ray, movePlane);

        if (m_projectedCursorPositionLastFrame.has_value() && projectedCursorPosition.has_value())
        {
            const auto delta = Math::dot(*projectedCursorPosition - *m_projectedCursorPositionLastFrame, gizmoAxisDirection);
            auto selectionTransform = world.editComponent<TransformComponent>(firstSelectedEntity);
            selectionTransform->position += gizmoAxisDirection * delta;
            m_projectedCursorPositionLastFrame = *projectedCursorPosition;
        }
        m_projectedCursorPositionLastFrame = projectedCursorPosition;
    } else
    {
        m_selectedGizmoAxis = {};
        m_projectedCursorPositionLastFrame = {};
    }

    if (Input::isKeyJustPressed(KeyCode::MouseButtonLeft))
    {
        const Ray ray = Engine::getViewportCursorRay(context().viewportId);
        m_selectedGizmoAxis = Physics::lineTrace(editorWorld, ray, TraceChannelFlags::Gizmo);
    }
}
