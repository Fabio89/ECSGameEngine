export module Editor.Panels.Viewport;
import Editor.Controller;
import ImGui;
import Editor.Panel.Impl;
import Editor.SelectionGizmo;
import Editor.SnapshotFrame;
import Editor.TransformTool;
import EventBus;
import std;

struct ViewportSnapshot
{
    Entity hitEntity;
};

class ViewportController : public EditorControllerImpl<ViewportSnapshot>
{
public:
    explicit ViewportController(EditingContext& context);

    void update(float dt, Editor::SnapshotFrame& frame) override;

    [[nodiscard]] TransformToolManager& tools() { return m_tools; }
    [[nodiscard]] const TransformToolManager& tools() const { return m_tools; }

private:
    ViewportSnapshot buildSnapshot(const EditingContext& context) override;

    TransformToolManager m_tools;
    SelectionGizmoManager m_selectionGizmos;
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

        std::reference_wrapper<ViewportController> m_controller;
        EventSubscription m_sub;
        bool m_open{true};
    };
}
