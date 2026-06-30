module Editor;
import Editor.Camera;
import Editor.Components;
import Editor.Controller;
import Editor.Events;
import ImGui;
import Editor.Panels.Hierarchy;
import Editor.Panels.Details;
import Editor.Panels.MainMenu;
import Editor.Panels.Viewport;
import Editor.PropertyDrawers;
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

    void execute(ChangeSelection&& request)
    {
        contexts().get(request.contextId).selection.set(request.entities);
    }

    void execute(OpenProject&& request)
    {
        Engine::openProject(std::move(request.path));
    }

    void execute(SetProperty&& request)
    {
        World& world = contexts().get(request.contextId).world.get();
        if (!world.isValid(request.entity))
            return;

        if (!world.hasComponent(request.entity, request.componentType))
            return;

        request.property->set(&world.editComponent(request.entity, request.componentType), request.value);
    }
}

void Editor::init(EditorUIContext context)
{
    EditorComponents::init();
    editorContext = context;

    initPropertyDrawers();

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
        std::visit([]<typename Request>(Request&& r) { execute(std::forward<Request>(r)); }, std::move(request));
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
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::ImVec4{0, 0, 0, 0});

    ImGui::DockSpace
    (
        ImGui::GetID("MainDockSpace"),
        ImGui::ImVec2(0, 0),
        ImGuiDockNodeFlags_PassthruCentralNode
    );

    ImGui::PopStyleColor();
    ImGui::End();

    for (auto& panel : panels)
    {
        panel->draw();
    }
}
