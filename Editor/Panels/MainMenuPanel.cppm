export module Editor.Panels.MainMenu;
import Editor;
import Editor.Controller;
import ImGui;
import Editor.Panel.Impl;
import Editor.Requests;
import FileSystem;
import World;

namespace Requests
{
    struct OpenProject
    {
        std::filesystem::path path;
    };

    struct OpenScene
    {
        std::filesystem::path path;
    };
}

using MainMenuRequest = std::variant<
    Requests::OpenProject,
    Requests::OpenScene
>;

class MainMenuController : public EditorControllerImpl<MainMenuController, NoController::Snapshot, MainMenuRequest>
{
public:
    using EditorControllerImpl::EditorControllerImpl;

    void execute(Requests::OpenProject&& request);

    void execute(Requests::OpenScene&& request);
};

export namespace Panels
{
    class MainMenuPanel : public PanelImpl<MainMenuController>
    {
    public:
        explicit MainMenuPanel(const PanelCreateInfo& info) : PanelImpl{info}
        {
            createController();
        }

    private:
        void doDraw() override;

        void drawFileMenu();
        void drawEditMenu();
        void drawViewMenu();
        void drawToolsMenu();
        void drawHelpMenu();
    };
}
