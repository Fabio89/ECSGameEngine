module Editor.Panels.MainMenu;
import Editor;
import Editor.Project;

void MainMenuController::execute(Requests::OpenProject&& request)
{
    Editor::openProject(context().id, std::move(request.path));
}

void MainMenuController::execute(Requests::OpenScene&& request)
{
    services().scenes.loadScene(context().world, request.path);
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
            request(Requests::OpenProject{std::move(*path)});
        }
    }

    if (ImGui::MenuItem("Open Scene..."))
    {
        if (const auto path = FileSystem::openFileDialog("*.scene"))
        {
            request(Requests::OpenScene{std::move(*path)});
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