export module Editor.Panels.Viewport;
import Editor.Controller;
import Editor.Panel.Impl;
import Editor.SelectionGizmo;
import Editor.Services;
import Editor.SnapshotFrame;
import Editor.TransformTool;
import Engine.Viewport;
import EventBus;
import ImGui;
import Core;

struct ViewportSnapshot
{
    Entity hitEntity;
};

template<>
constexpr std::string_view getTypeName<ViewportSnapshot>() { return "ViewportSnapshot"; }

class ViewportController : public EditorControllerImpl<ViewportSnapshot>
{
public:
    explicit ViewportController(EditorServices& services, EditingContext& context);

    void update(float dt, Editor::SnapshotFrame& frame) override;

    [[nodiscard]] TransformToolManager& tools() { return m_tools; }
    [[nodiscard]] const TransformToolManager& tools() const { return m_tools; }

private:
    ViewportSnapshot buildSnapshot(const EditingContext& context) override;

    TransformToolManager m_tools;
    SelectionGizmoManager m_selectionGizmos;
    EventSubscription m_subscription;
};

export namespace Panels
{
    class ViewportPanel : public PanelImpl
    {
    public:
        explicit ViewportPanel(const PanelCreateInfo& info);
        static constexpr auto Name = "Viewport";

    private:
        void doDraw() override;
        void setCurrentTool(EntityEditingMode type);
        void drawFpsCounter() const;

        ViewportId m_id;
        EventSubscription m_sub;
        bool m_open{true};
    };
}
