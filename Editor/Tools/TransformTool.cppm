export module Editor.TransformTool;
export import Editor.EntityEditingMode;
import Editor.EditingContext;
import Editor.Services;
import Engine.Viewport;
import EventBus;
import Math;
import Render.Viewport;
import Window;

export struct TransformToolContext
{
    EditorServices& services;
    EditingContext& editing;
    const ViewportId& viewportId;
    const WindowHandle& window;
};

export class TransformToolManager : EditorServiceConsumer
{
public:
    explicit TransformToolManager(TransformToolContext context);
    void createTools();
    void setCurrentTool(EntityEditingMode type);
    void update();
    bool isSelectionEnabled() const;

private:
    TransformToolContext m_context;
    EntityEditingMode m_currentToolType{EntityEditingMode::None};
    std::array<std::unique_ptr<class TransformTool>, 3> m_tools;
    EventSubscription m_subscription;
};

export class TransformTool : EditorServiceConsumer
{
public:
    TransformTool(TransformToolContext& context, EntityEditingMode type);
    virtual ~TransformTool() = default;
    virtual void update();
    void setActive(bool active);
    bool isSelectionEnabled() const;

protected:
    void setSelectionEnabled(bool enabled);
    TransformToolContext& context() { return m_context; }
    [[nodiscard]] const TransformToolContext& context() const { return m_context; }

private:
    void attachToSelection();

    TransformToolContext& m_context;
    Entity m_attachedTo;
    Entity m_gizmo;
    bool m_selectionEnabled{true};
};

export template<EntityEditingMode mode>
class TransformToolImpl : public TransformTool
{
public:
    explicit TransformToolImpl(TransformToolContext& context)
        : TransformTool{context, mode} {}
};