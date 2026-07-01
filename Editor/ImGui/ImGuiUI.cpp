module Editor.ImGuiUI;
import Editor;
import Editor.Panel;
import Editor.Panels.Details;
import Editor.Panels.Hierarchy;
import Editor.Panels.Viewport;
import ImGui;
import std;

namespace
{
    constexpr char MainDockSpaceName[] = "MainDockSpace";
    const std::filesystem::path imguiIniPath{"Editor/imgui.ini"};
    bool firstFrame = true;
}

namespace Editor::ImGuiUI
{
    void createDefaultLayout();
}

void Editor::ImGuiUI::init()
{
    ImGui::StyleColorsDark();

    ImGuiIO& io = ImGui::GetIO();

    io.IniFilename = imguiIniPath.c_str();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Set fonts
    {
        ImFontConfig config;
        config.OversampleH = 2;
        config.OversampleV = 2;

        io.Fonts->AddFontFromFileTTF(
            "Editor/Assets/Fonts/NotoSans-Variable.ttf",
            18.0f,
            &config
        );
        io.FontDefault = io.Fonts->Fonts.back();
    }
}

void Editor::ImGuiUI::draw()
{
    const ImGui::ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    static constexpr ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus |
            ImGuiWindowFlags_NoBackground;

    ImGui::Begin("DockSpace", nullptr, flags);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{0, 0, 0, 0});

    ImGui::DockSpace
    (
        ImGui::GetID(MainDockSpaceName),
        ImVec2(0, 0),
        ImGuiDockNodeFlags_PassthruCentralNode
    );

    if (firstFrame)
    {
        firstFrame = false;

        if (!std::filesystem::exists(imguiIniPath))
            createDefaultLayout();
    }

    ImGui::PopStyleColor();
    ImGui::End();

    for (Panel* panel : getPanels())
    {
        panel->draw();
    }
}

void Editor::ImGuiUI::createDefaultLayout()
{
    const ImGuiID dockSpace = ImGui::GetID(MainDockSpaceName);

    ImGui::DockBuilderRemoveNode(dockSpace);
    ImGui::DockBuilderAddNode(dockSpace, ImGuiDockNodeFlags_DockSpace);

    ImGui::DockBuilderSetNodeSize(dockSpace, ImGui::GetMainViewport()->Size);

    // Create right column
    ImGuiID rightNode;
    ImGuiID mainNode = dockSpace;

    ImGui::DockBuilderSplitNode(
        mainNode,
        ImGuiDir_Right,
        0.25f,
        &rightNode,
        &mainNode
    );

    // Split right column vertically
    ImGuiID detailsNode;
    ImGuiID hierarchyNode = rightNode;

    ImGui::DockBuilderSplitNode(
        hierarchyNode,
        ImGuiDir_Down,
        0.5f,
        &detailsNode,
        &hierarchyNode
    );

    ImGui::DockBuilderDockWindow(Panels::HierarchyPanel::Name, hierarchyNode);

    ImGui::DockBuilderDockWindow(Panels::DetailsPanel::Name, detailsNode);

    ImGui::DockBuilderDockWindow(Panels::ViewportPanel::Name, mainNode);

    ImGui::DockBuilderFinish(dockSpace);
}
