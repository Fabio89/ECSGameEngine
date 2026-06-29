module Editor;
import Editor.Camera;
import Editor.Components;
import Editor.Controller;
import Editor.Events;
import Editor.ImGui;
import Editor.Panels.Hierarchy;
import Editor.Panels.Details;
import Editor.Panels.MainMenu;
import Editor.Panels.Viewport;
import Engine;
import Input;
import Math;
import Physics;
import Thread;
import World;
import World.Events;

namespace
{
    EventSubscription subscription;
    std::vector<std::unique_ptr<Panel>> panels;
}

namespace Editor
{
    void draw();

    void execute(const Requests::ChangeSelection& event)
    {
        contexts().get(event.contextId).selection.set(event.entities);
    }
}

void Editor::init(EditorUIContext context)
{
    EditorComponents::init();
    editorContext = context;

    const EditingContextId defaultContextId = contexts().add(*context.world);
    addPanel<Panels::HierarchyPanel>(defaultContextId);
    addPanel<Panels::DetailsPanel>(defaultContextId);
    addPanel<Panels::MainMenuPanel>(defaultContextId);
    addPanel<Panels::ViewportPanel>(defaultContextId);

    Engine::setEditorDrawCallback(draw);

    controllerManager.init();

    subscription += Engine::events().subscribe([](const Engine::SceneLoadedEvent& event)
    {
        for (EditingContext& context : contexts().getAll())
        {
            if (&context.world.get() == &event.world.get())
            {
                context.selection.clear();
            }
        }
    });
}

void Editor::shutdown()
{
    subscription = {};
    panels.clear();
    controllerManager = {};
}

void Editor::update(float deltaTime)
{
    EditorRequest request;

    while (requests.tryPop(request))
    {
        std::visit(execute, request);
    }

    controllerManager.update(deltaTime);

    EditorCamera::update(editorContext.window, *editorContext.world, Engine::getPlayer(), deltaTime);
}

void Editor::addPanel(std::unique_ptr<Panel> panel)
{
    panels.emplace_back(std::move(panel));
}

void Editor::draw()
{
    ImGui::ImGuiViewport* viewport = ImGui::GetMainViewport();

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

    ImGui::DockSpace
    (
        ImGui::GetID("MainDockSpace"),
        ImGui::ImVec2(0, 0),
        ImGuiDockNodeFlags_PassthruCentralNode
    );

    ImGui::End();

    for (auto& panel : panels)
    {
        panel->draw();
    }
}
