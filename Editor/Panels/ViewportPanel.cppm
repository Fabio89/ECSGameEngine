export module Editor.Panels.Viewport;
import Core;
import Editor.Controller;
import Editor.Panel.Impl;
import Editor.SelectionGizmo;
import Editor.Services;
import Editor.SnapshotFrame;
import Editor.TransformTool;
import Engine.Viewport;
import EventBus;
import ImGui;
import Window;

struct ViewportSnapshot
{
    bool selectionEnabled{};
};

namespace Requests
{
    struct AssignViewport
    {
        ViewportId viewportId;
    };

    struct HandleMouseSelect
    {
    };

    struct SetCameraMouseLookEnabled
    {
        bool enabled{};
    };

    struct SetEditingMode
    {
        EntityEditingMode mode{EntityEditingMode::None};
    };
}

using ViewportRequest = std::variant<
    Requests::AssignViewport,
    Requests::HandleMouseSelect,
    Requests::SetCameraMouseLookEnabled,
    Requests::SetEditingMode
>;

class ViewportController : public EditorControllerImpl<ViewportController, ViewportSnapshot, ViewportRequest>
{
public:
    explicit ViewportController(EditorServices& services, EditingContext& context, SharedMailbox mailbox, WindowHandle window);

    void update(float dt, Editor::SnapshotFrame& frame) override;

    [[nodiscard]] TransformToolManager& tools() { return m_tools; }
    [[nodiscard]] const TransformToolManager& tools() const { return m_tools; }

    void execute(Requests::AssignViewport&& request);
    void execute(Requests::SetCameraMouseLookEnabled&& request);
    void execute(Requests::HandleMouseSelect&& request);
    void execute(Requests::SetEditingMode&& request);

private:
    ViewportSnapshot buildSnapshot(const EditingContext& context) override;

    TransformToolManager m_tools;
    SelectionGizmoManager m_selectionGizmos;
    EventSubscription m_subscription;
    Entity m_camera;
    ViewportId m_viewportId;
    WindowHandle m_window;
};

export namespace Panels
{
    class ViewportPanel : public PanelImpl<ViewportController>
    {
    public:
        explicit ViewportPanel(const PanelCreateInfo& info);
        static constexpr auto Name = "Viewport";

    private:
        void doDraw() override;
        void setCurrentTool(EntityEditingMode type);
        void drawFpsCounter() const;

        ViewportId m_viewportId;
        EventSubscription m_subscription;
        bool m_open{true};
    };
}
