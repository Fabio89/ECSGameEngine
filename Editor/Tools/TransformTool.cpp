module Editor.TransformTool;
import Component.Gizmo;
import Editor;
import Editor.Events;
import Editor.Gizmos;
import Editor.Selection;
import Engine;
import Input;
import Math;
import Physics;
import World;

TransformToolManager::TransformToolManager(EditingContext& context) : m_context{context}
{
    m_sub += Engine::events().subscribe([this](const Editor::SelectionChangedEvent& event)
    {
        if (m_currentToolType != EntityEditingMode::None)
        {
            TransformTool& currentTool = *m_tools[static_cast<std::size_t>(m_currentToolType)];
            if (event.selection.empty())
            {

            }
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
        if (m_context.get().selection.isEmpty())
            currentTool.setActive(false);
        else
            currentTool.update();
    }
}

TransformTool::TransformTool(EditingContext& context, EntityEditingMode type)
    : m_context{context},
      m_gizmo{Gizmos::createTransformGizmo(Engine::getWorld(context.world), type)} {}

void TransformTool::update()
{
    attachToSelection();
}

void TransformTool::setActive(bool active)
{
    World& world = Engine::getWorld(m_context.get().world);
    if (!active)
        Gizmos::setGizmoVisible(world, m_gizmo, false);

    if (world.hasComponent<GizmoComponent>(m_gizmo))
    {
        auto setAxis = [active, &world](Entity axis)
        {
            if (!axis.isValid()) return;
            auto& collision = world.editComponent<BoundingBoxComponent>(axis);
            collision.channel.set(TraceChannelFlags::Gizmo, active);
        };

        auto& gizmoComponent = world.editComponent<GizmoComponent>(m_gizmo);

        setAxis(gizmoComponent.xAxisEntity);
        setAxis(gizmoComponent.yAxisEntity);
        setAxis(gizmoComponent.zAxisEntity);
    }

    if (active)
    {
        attachToSelection();
        Gizmos::setGizmoVisible(world, m_gizmo, true);
    }
}

void TransformTool::attachToSelection()
{
    Entity firstSelected = context().selection.isEmpty() ? Entity{} : context().selection.get().front();
    if (firstSelected.isValid() && firstSelected != m_attachedTo)
    {
        World& world = Engine::getWorld(m_context.get().world);
        HierarchyUtils::setParent(world, m_gizmo, firstSelected);
        world.editComponent<TransformComponent>(m_gizmo).scale = 0.2f / world.readComponent<TransformComponent>(firstSelected).scale;
    }
}

void TranslateTool::update()
{
    TransformTool::update();

    World& world = Engine::getWorld(context().world);

    if (context().selection.isEmpty())
        return;

    Entity firstSelectedEntity = context().selection.get().front();
    if (world.isValid(firstSelectedEntity) && world.isValid(m_selectedGizmoAxis) && Input::isKeyDown(KeyCode::MouseButtonLeft))
    {
        TransformComponent& transform = world.editComponent<TransformComponent>(firstSelectedEntity);

        const Vec3 gizmoAxisDirection = [&]
        {
            const Entity gizmoEntity = HierarchyUtils::getParent(world, m_selectedGizmoAxis);
            const GizmoComponent& gizmo = world.readComponent<GizmoComponent>(gizmoEntity);
            if (m_selectedGizmoAxis == gizmo.xAxisEntity)
                return TransformUtils::right(transform);
            if (m_selectedGizmoAxis == gizmo.yAxisEntity)
                return TransformUtils::up(transform);
            if (m_selectedGizmoAxis == gizmo.zAxisEntity)
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

        const Ray ray = Engine::getViewportCursorRay(world);
        const std::optional<Vec3> projectedCursorPosition = Physics::intersectRayPlane(ray, movePlane);

        if (m_projectedCursorPositionLastFrame.has_value() && projectedCursorPosition.has_value())
        {
            const auto delta = Math::dot(*projectedCursorPosition - *m_projectedCursorPositionLastFrame,
                                         gizmoAxisDirection);
            transform.position += gizmoAxisDirection * delta;
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
        const Ray ray = Engine::getViewportCursorRay(world);
        m_selectedGizmoAxis = Physics::lineTrace(world, ray, TraceChannelFlags::Gizmo);
    }
}
