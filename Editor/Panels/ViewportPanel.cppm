export module Editor.Panels.Viewport;
import Editor.Controller;
import ImGui;
import Editor.Panel.Impl;
import Editor.SelectionGizmo;
import Editor.TransformTool;
import EventBus;
import std;

class ViewportController;

export namespace Panels
{
    class ViewportPanel : public PanelImpl
    {
    public:
        explicit ViewportPanel(const PanelCreateInfo& info);

    private:
        void doDraw() override;
        void setCurrentTool(EntityEditingMode type);

        std::reference_wrapper<ViewportController> m_controller;
        EventSubscription m_sub;
        bool m_open{true};
    };
}

class ViewportController : public EditorController
{
public:
    explicit ViewportController(EditingContext& context);

    void update(float dt) override;

    [[nodiscard]] TransformToolManager& tools() { return m_tools; }
    [[nodiscard]] const TransformToolManager& tools() const { return m_tools; }

private:
    TransformToolManager m_tools;
    SelectionGizmoManager m_selectionGizmos;
};
