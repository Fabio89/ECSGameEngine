export module Editor.Panel.ImGuiDemo;
import Editor.ImGui;
import Editor.Panel;
import World;

export namespace Panels
{
    class ImGuiDemo : public Panel
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
