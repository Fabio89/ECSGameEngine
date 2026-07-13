export module Editor.TransformTool;
export import Editor.EntityEditingMode;
import Editor.EditingContext;
import Editor.Services;
import EventBus;
import Math;

export class TransformToolManager : EditorServiceConsumer
{
public:
    TransformToolManager(EditorServices& services, EditingContext& context);
    void createTools();
    void setCurrentTool(EntityEditingMode type);
    void update();

private:
    EditingContext& m_context;
    EntityEditingMode m_currentToolType{EntityEditingMode::None};
    std::array<std::unique_ptr<class TransformTool>, 3> m_tools;
    EventSubscription m_sub;
};

class TransformTool : EditorServiceConsumer
{
public:
    TransformTool(EditorServices& services, EditingContext& context, EntityEditingMode type);
    virtual ~TransformTool() = default;
    virtual void update();
    void setActive(bool active);

protected:
    EditingContext& context() { return m_context; }
    [[nodiscard]] const EditingContext& context() const { return m_context; }

private:
    void attachToSelection();

    EditingContext& m_context;
    Entity m_attachedTo;
    Entity m_gizmo;
};

template<EntityEditingMode mode>
class TransformToolImpl : public TransformTool
{
public:
    explicit TransformToolImpl(EditorServices& services, EditingContext& context) : TransformTool{services, context, mode} {}
};

class TranslateTool : public TransformToolImpl<EntityEditingMode::Translate>
{
public:
    using TransformToolImpl::TransformToolImpl;
    void update() override;

private:
    Entity m_selectedGizmoAxis;
    std::optional<Vec3> m_projectedCursorPositionLastFrame;
};

class RotateTool : public TransformToolImpl<EntityEditingMode::Rotate>
{
public:
    using TransformToolImpl::TransformToolImpl;
};

class ScaleTool : public TransformToolImpl<EntityEditingMode::Scale>
{
public:
    using TransformToolImpl::TransformToolImpl;
};