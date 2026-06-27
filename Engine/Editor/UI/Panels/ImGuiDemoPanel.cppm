export module UI.Panel.ImGuiDemo;
import UI.Panel;

export namespace Panels
{
    class ImGuiDemoPanel : public Panel
    {
    public:
        using Panel::Panel;

    private:
        void doDraw(World&) override
        {
            if (m_showDemoWindow)
                ImGui::ShowDemoWindow(&m_showDemoWindow);
        }

        bool m_showDemoWindow{true};
    };
}
