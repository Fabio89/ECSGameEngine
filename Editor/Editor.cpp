module Editor;
import AssetManager;
import Editor.Camera;
import Editor.Components;
import Editor.Config;
import Editor.Controller;
import Editor.Events;
import Editor.ImGuiUI;
import Editor.Panels.Hierarchy;
import Editor.Panels.Details;
import Editor.Panels.MainMenu;
import Editor.Panels.Viewport;
import Editor.Project;
import Editor.PropertyDrawers;
import Editor.Services;
import Engine;
import Input;
import Math;
import Physics;
import Systems.BoundingBox;
import Systems.EntityProxy;
import Systems.Hierarchy;
import Systems.RenderSynchronizer;
import Systems.Transform;
import Thread;
import World;
import World.Events;
import Editor.Gizmos;
import Editor.EntityEditingMode;

EditorUIContext editorContext;
EventSubscription subscription;
std::vector<std::unique_ptr<Panel>> panels;
std::vector<Panel*> panelView;
EventBus events;
AssetMountId projectAssetsMount;
AssetMountId editorAssetsMount;
ThreadSafeQueue<EditorRequest> requests;

EditorServices services
{
    .worlds = Engine::worlds(),
    .viewports = Engine::viewports(),
    .scenes = Engine::scenes(),
    .assets = Engine::assets(),
    .events = events,
    .renderCommands = Engine::getRenderCommandQueue()
};

EditingContextManager contextManager{services.worlds, services.events};
std::unordered_map<EditingContextId, Editor::ControllerManager> controllerManagers;

namespace Editor
{
    void rebuildPanelView();
    void loadScene(EditingContextId contextId, const std::filesystem::path& path);
    void init();
    void shutdown();
    bool update();

    void execute(AddController&& request)
    {
        ensureControllerManager(request.contextId).addController(request.factory(services, contexts().get(request.contextId)));
    }
}

void Editor::request(EditorRequest request)
{
    requests.push(std::move(request));
}

PanelCreateInfo Editor::generatePanelInfo(EditingContextId contextId)
{
    return {
        .contextId = contextId,
        .window = editorContext.window
    };
}

void Editor::addPanel(std::unique_ptr<Panel> panel)
{
    panels.emplace_back(std::move(panel));
    rebuildPanelView();
}

Editor::ControllerManager& Editor::ensureControllerManager(EditingContextId contextId)
{
    return controllerManagers.try_emplace(contextId, services).first->second;
}

void Editor::init()
{
    Engine::addSystem(EntityProxySystem::callbacks);
    Engine::addSystem(HierarchySystem::callbacks);
    Engine::addSystem(TransformSystem::callbacks);
    Engine::addSystem(BoundingBoxSystem::callbacks);
    Engine::addSystem(RenderSynchronizer::callbacks);

    Engine::init();

    editorAssetsMount = services.assets.mount("Editor", "Editor/Assets");
    Gizmos::init(services.assets, editorAssetsMount);

    EditorComponents::init();

    initPropertyDrawers();

    const WorldHandle editorWorld = Engine::createWorld();
    const WorldHandle mainWorld = Engine::createWorld();

    editorContext = {.world = mainWorld, .window = Engine::getWindow()};

    const EditingContextId defaultContextId = contexts().add({editorContext.world, editorWorld});

    addPanel<Panels::HierarchyPanel>(defaultContextId);
    addPanel<Panels::DetailsPanel>(defaultContextId);
    addPanel<Panels::MainMenuPanel>(defaultContextId);
    addPanel<Panels::ViewportPanel>(defaultContextId);

    Engine::setEditorCallbacks({
        .imguiInit = ImGuiUI::init,
        .draw = ImGuiUI::draw
    });

    for (ControllerManager& controllerManager : controllerManagers | std::views::values)
        controllerManager.init();

    subscription += services.worlds.subscribe([](const WorldEvents::WorldCleared& event)
    {
        for (EditingContext& context : contexts().getAll())
        {
            if (&context.world == &event.world)
            {
                context.selection.clear();
            }
        }
    });

    if (EditorConfig config = loadEditorConfig(); !config.lastProject.empty())
    {
        if (std::filesystem::exists(config.lastProject))
            openProject(defaultContextId, config.lastProject);
        else
        {
            config.lastProject = "";
            saveEditorConfig(config);
        }
    }
    Engine::start();
}

void Editor::shutdown()
{
    subscription = {};
    panels.clear();
    controllerManagers.clear();
    Engine::shutdown();
}

bool Editor::update()
{
    EditorRequest request;

    while (requests.tryPop(request))
        std::visit([]<typename Request>(Request&& r) { execute(std::forward<Request>(r)); }, std::move(request));

    for (auto& [contextId, controllerManager] : controllerManagers)
        controllerManager.update(Engine::getSimulationDeltaTime(), contexts().get(contextId).snapshotPublisher);

    if (!Engine::update())
        return false;

    return true;
}

void Editor::run()
{
    init();

    while (update()) {}

    shutdown();
}

std::span<Panel*> Editor::getPanels()
{
    return panelView;
}

EditingContextManager& Editor::contexts() { return contextManager; }

void Editor::openProject(EditingContextId contextId, const std::filesystem::path& path)
{
    const ProjectConfig projectConfig = loadProjectConfig(path / "project.toml");

    if (projectAssetsMount.isValid())
        services.assets.unmount(projectAssetsMount);
    projectAssetsMount = services.assets.mount("Project", path / projectConfig.contentRoot);
    services.assets.loadDatabase(projectAssetsMount, path / projectConfig.assetDatabase);

    loadScene(contextId, path / projectConfig.startupScene);

    EditorConfig editorConfig = loadEditorConfig();
    editorConfig.lastProject = path;
    saveEditorConfig(editorConfig);
}

void Editor::rebuildPanelView()
{
    panelView.clear();

    for (auto& panel : panels)
        panelView.push_back(panel.get());
}

void Editor::loadScene(EditingContextId contextId, const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path))
        return;

    services.scenes.loadScene(contexts().get(contextId).world, path);
}
