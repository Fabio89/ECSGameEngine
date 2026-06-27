module EditorUI;
import UI.ImGui;

void EditorUI::addPanel(std::unique_ptr<IPanel> panel)
{
    m_panels.emplace_back(std::move(panel));
}

void EditorUI::draw()
{
    ImGui::ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    static constexpr ImGui::ImGuiWindowFlags flags =
        ImGui::ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar |
        ImGui::ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse |
        ImGui::ImGuiWindowFlags_::ImGuiWindowFlags_NoResize |
        ImGui::ImGuiWindowFlags_::ImGuiWindowFlags_NoMove |
        ImGui::ImGuiWindowFlags_::ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGui::ImGuiWindowFlags_::ImGuiWindowFlags_NoNavFocus |
        ImGui::ImGuiWindowFlags_::ImGuiWindowFlags_NoBackground;

    ImGui::Begin("DockSpace", nullptr, flags);

    ImGui::DockSpace
    (
        ImGui::GetID("MainDockSpace"),
        ImGui::ImVec2(0, 0),
        ImGui::ImGuiDockNodeFlags_::ImGuiDockNodeFlags_PassthruCentralNode
    );

    ImGui::End();

    for (auto& panel : m_panels)
    {
        panel->draw();
    }
}

bool EditorUI::isMouseAvailable()
{
    return !ImGui::GetCurrentContext() || !ImGui::GetIO().WantCaptureMouse;
}

bool EditorUI::isKeyboardAvailable()
{
    return !ImGui::GetCurrentContext() || !ImGui::GetIO().WantCaptureKeyboard;
}

