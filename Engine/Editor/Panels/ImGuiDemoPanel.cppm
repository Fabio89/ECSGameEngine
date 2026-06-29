export module Editor.Panels.ImGuiDemo;
import Editor.ImGui;
import Editor.Panel;

export namespace Panels
{
    class ImGuiDemoPanel : public Panel
    {
    public:
        using Panel::Panel;

    private:
        void doDraw() override
        {
            if (m_showDemoWindow)
                ImGui::ShowDemoWindow(&m_showDemoWindow);
        }

        bool m_showDemoWindow{true};
    };
}
