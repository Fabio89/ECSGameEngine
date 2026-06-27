export module Editor.Panel.Viewport;
import Editor.ImGui;
import Editor.Panel;
import World;

export namespace Panels
{
    class Viewport : public Panel
    {
    public:
        using Panel::Panel;

    private:
        void doDraw(World&) override
        {

        }
    };
}
