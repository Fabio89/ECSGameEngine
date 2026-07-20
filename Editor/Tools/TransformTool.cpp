module Editor.TransformTool;
import :Detail;
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
    m_subscription += m_context.services.events.subscribe([this](const EditorEvents::SelectionChanged& event)
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

bool TransformToolManager::isSelectionEnabled() const
{
    if (m_currentToolType == EntityEditingMode::None)
        return true;

    const TransformTool& currentTool = *m_tools[static_cast<std::size_t>(m_currentToolType)];
    return currentTool.isSelectionEnabled();
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
    {
        setSelectionEnabled(true);
        Gizmos::setGizmoVisible(editorWorld, m_gizmo, false);
    }

    if (editorWorld.hasComponent<GizmoComponent>(m_gizmo))
    {
        auto setHandle = [active, &editorWorld](Entity handle)
        {
            if (!handle.isValid()) return;
            auto collision = editorWorld.editComponent<BoundingBoxComponent>(handle);
            collision->channel.set(TraceChannelFlags::Gizmo, active);
        };

        auto& gizmoComponent = editorWorld.readComponent<GizmoComponent>(m_gizmo);
        GizmoUtils::forEachHandle(gizmoComponent, setHandle);
    }

    if (active)
    {
        attachToSelection();
        Gizmos::setGizmoVisible(editorWorld, m_gizmo, !context().editing.selection.isEmpty());
    }
}

bool TransformTool::isSelectionEnabled() const
{
    return m_selectionEnabled;
}

void TransformTool::setSelectionEnabled(bool enabled)
{
    m_selectionEnabled = enabled;
}

float calculateGizmoScale
(
    const Camera& camera,
    Vec3 worldPosition,
    float desiredScreenSize
)
{
    const Vec4 viewPosition = camera.view * Vec4(worldPosition, 1.0f);
    const float depth = -viewPosition.z;
    const float scale = desiredScreenSize * depth / camera.proj[1][1];
    return scale;
}

void TransformTool::attachToSelection()
{
    if (!context().viewportId)
        return;

    Entity firstSelected = context().editing.selection.isEmpty() ? Entity{} : context().editing.selection.get().front();
    if (firstSelected.isValid())
    {
        const World& mainWorld = Engine::getWorld(m_context.editing.world);
        World& editorWorld = Engine::getWorld(m_context.editing.editorWorld);

        auto gizmoTransform = editorWorld.editComponent<TransformComponent>(m_gizmo);
        const auto& selectionTransform = mainWorld.readComponent<TransformComponent>(firstSelected);

        gizmoTransform->position = selectionTransform.position;
        gizmoTransform->rotation = selectionTransform.rotation;

        const Camera& camera = services().viewports.getCamera(context().viewportId);
        gizmoTransform->scale = Vec3{calculateGizmoScale(camera, gizmoTransform->position, 0.12f)};
    }
    m_attachedTo = firstSelected;
}
