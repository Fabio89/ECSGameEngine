export module Editor.Panels.MainMenu;
import Editor;
import ImGui;
import Editor.Panel;
import Editor.Requests;
import Engine;
import FileSystem;
import World;

export namespace Panels
{
    class MainMenuPanel : public Panel
    {
    public:
        using Panel::Panel;

    private:
        void doDraw() override;

        void drawFileMenu();
        void drawEditMenu();
        void drawViewMenu();
        void drawToolsMenu();
        void drawHelpMenu();
    };
}

void Panels::MainMenuPanel::doDraw()
{
    if (!ImGui::BeginMainMenuBar())
        return;

    drawFileMenu();
    drawEditMenu();
    drawViewMenu();
    drawToolsMenu();
    drawHelpMenu();

    ImGui::EndMainMenuBar();
}

void Panels::MainMenuPanel::drawFileMenu()
{
    if (!ImGui::BeginMenu("File"))
        return;

    if (ImGui::MenuItem("Open Project..."))
    {
        if (const auto path = FileSystem::openFolderDialog())
        {
            Editor::request(Editor::OpenProject{std::move(*path)});
        }
    }

    ImGui::Separator();

    if (ImGui::MenuItem("Exit"))
    {
        // TODO
        // Application::quit();
    }

    ImGui::EndMenu();
}

void Panels::MainMenuPanel::drawEditMenu() {}
void Panels::MainMenuPanel::drawViewMenu() {}
void Panels::MainMenuPanel::drawToolsMenu() {}
void Panels::MainMenuPanel::drawHelpMenu() {}
