export module UI.Panel.MainMenu;
import Engine;
import FileSystem;
import UI.Panel;

export namespace Panels
{
    class MainMenuPanel : public Panel
    {
    public:
        using Panel::Panel;

    private:
        void doDraw(World& world) override;

        void drawFileMenu(World& world);
        void drawEditMenu(World& world);
        void drawViewMenu(World& world);
        void drawToolsMenu(World& world);
        void drawHelpMenu(World& world);
    };
}

void Panels::MainMenuPanel::doDraw(World& world)
{
    if (!ImGui::BeginMainMenuBar())
        return;

    drawFileMenu(world);
    drawEditMenu(world);
    drawViewMenu(world);
    drawToolsMenu(world);
    drawHelpMenu(world);

    ImGui::EndMainMenuBar();
}

void Panels::MainMenuPanel::drawFileMenu(World& world)
{
    if (!ImGui::BeginMenu("File"))
        return;

    if (ImGui::MenuItem("Open Project..."))
    {
        if (const auto path = FileSystem::openFileDialog())
        {
            Engine::openProject(*path);
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

void Panels::MainMenuPanel::drawEditMenu(World& world) {}
void Panels::MainMenuPanel::drawViewMenu(World& world) {}
void Panels::MainMenuPanel::drawToolsMenu(World& world) {}
void Panels::MainMenuPanel::drawHelpMenu(World& world) {}
