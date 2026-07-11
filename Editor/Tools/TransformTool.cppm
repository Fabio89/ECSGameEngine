export module Editor.TransformTool;
export import Editor.EntityEditingMode;
import Editor.EditingContext;
import Editor.Services;
import EventBus;
import Math;

export class TransformToolManager
{
public:
    TransformToolManager(EditorServices& services, EditingContext& context);
    void createTools();
    void setCurrentTool(EntityEditingMode type);
    void update();

private:
    EditorServices& m_services;
    EditingContext& m_context;
    EntityEditingMode m_currentToolType{EntityEditingMode::None};
    std::array<std::unique_ptr<class TransformTool>, 3> m_tools;
    EventSubscription m_sub;
};

class TransformTool
{
public:
    TransformTool(EditingContext& context, EntityEditingMode type);
    virtual ~TransformTool() = default;
    virtual void update();
    void setActive(bool active);

protected:
    EditingContext& context() { return m_context; }
    [[nodiscard]] const EditingContext& getWorld() const { return m_context; }

private:
    void attachToSelection();

    EditingContext& m_context;
    Entity m_attachedTo;
    Entity m_gizmo;
};

class TranslateTool : public TransformTool
{
public:
    explicit TranslateTool(EditingContext& context) : TransformTool{context, EntityEditingMode::Translate} {}
    void update() override;

private:
    Entity m_selectedGizmoAxis;
    std::optional<Vec3> m_projectedCursorPositionLastFrame;
};

class RotateTool : public TransformTool
{
public:
    explicit RotateTool(EditingContext& context) : TransformTool{context, EntityEditingMode::Rotate} {}
};

class ScaleTool : public TransformTool
{
public:
    explicit ScaleTool(EditingContext& context) : TransformTool{context, EntityEditingMode::Scale} {}
};