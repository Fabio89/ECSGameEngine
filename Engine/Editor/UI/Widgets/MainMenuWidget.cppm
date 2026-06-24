export module UI.Widget.MainMenu;
import FileSystem;
import UI.ImGui;
import UI.Widget;
import World;

export namespace Widgets
{
    class MainMenuWidget : public Widget
    {
    public:
        using Widget::Widget;

    private:
        void doDraw(World& world) override;

        void drawFileMenu(World& world);
        void drawEditMenu(World& world);
        void drawViewMenu(World& world);
        void drawToolsMenu(World& world);
        void drawHelpMenu(World& world);
    };
}

void Widgets::MainMenuWidget::doDraw(World& world)
{
    if (!ImGui::BeginMainMenuBar())
        return;

    drawEditMenu(world);
    drawViewMenu(world);
    drawToolsMenu(world);
    drawHelpMenu(world);

    ImGui::EndMainMenuBar();
}

void Widgets::MainMenuWidget::drawFileMenu(World& world)
{
    if (!ImGui::BeginMenu("File"))
        return;

    if (ImGui::MenuItem("Open Project..."))
    {
        if (const auto path = FileSystem::openFileDialog(); path.has_filename())
        {

            // TODO
            // ProjectManager::load(*projectPath);
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

void Widgets::MainMenuWidget::drawEditMenu(World& world) {}
void Widgets::MainMenuWidget::drawViewMenu(World& world) {}
void Widgets::MainMenuWidget::drawToolsMenu(World& world) {}
void Widgets::MainMenuWidget::drawHelpMenu(World& world) {}
