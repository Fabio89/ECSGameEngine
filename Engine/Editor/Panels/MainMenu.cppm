export module Editor.Panel.MainMenu;
import Editor.ImGui;
import Editor.Panel;
import Engine;
import FileSystem;
import World;

export namespace Panels
{
    class MainMenu : public Panel
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

void Panels::MainMenu::doDraw(World& world)
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

void Panels::MainMenu::drawFileMenu(World& world)
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

void Panels::MainMenu::drawEditMenu(World& world) {}
void Panels::MainMenu::drawViewMenu(World& world) {}
void Panels::MainMenu::drawToolsMenu(World& world) {}
void Panels::MainMenu::drawHelpMenu(World& world) {}
