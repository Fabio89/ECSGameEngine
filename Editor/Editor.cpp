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

EventSubscription subscription;
std::vector<std::unique_ptr<Panel>> panels;
std::vector<Panel*> panelView;
EventBus events;

EditorServices services
{
    .worlds = Engine::worlds(),
    .viewports = Engine::viewports(),
    .scenes = Engine::scenes(),
    .events = events,
    .renderCommands = Engine::getRenderCommandQueue()
};

EditingContextManager contextManager{services.worlds, services.events};
std::unordered_map<EditingContextId, Editor::ControllerManager> controllerManagers;

namespace Editor
{
    void rebuildPanelView();
    Entity ensureCamera(World& world);
    void loadScene(EditingContextId contextId, const std::filesystem::path& path);
    void init();
    void shutdown();
    bool update();

    void execute(ChangeSelection&& request)
    {
        contexts().get(request.contextId).selection.set(request.entities);
    }

    void execute(SetEntityEditingMode&& request)
    {
        events.publish(EditorEvents::EntityEditingModeChanged{request.contextId, request.mode});
    }

    void execute(SelectEntityUnderCursor&& request)
    {
        check(request.window.isValid(), "");

        const World& world = services.worlds.get(contextManager.get(request.contextId).world);
        const Ray ray = Engine::getViewportCursorRay(request.viewport);
        const Entity hitEntity = Physics::lineTrace(world, ray, TraceChannelFlags::Default);

        execute(ChangeSelection{.contextId = request.contextId, .entities = {hitEntity}});
    }

    void execute(AddController&& request)
    {
        ensureControllerManager(request.contextId).addController(request.factory(services, contexts().get(request.contextId)));
    }

    void execute(OpenProject&& request)
    {
        const ProjectConfig projectConfig = loadProjectConfig(request.path / "project.toml");

        AssetManager::setContentRoot(request.path / projectConfig.contentRoot);
        AssetManager::loadDatabase(request.path / projectConfig.assetDatabase);

        loadScene(request.contextId, request.path / projectConfig.startupScene);

        EditorConfig editorConfig = loadEditorConfig();
        editorConfig.lastProject = request.path;
        saveEditorConfig(editorConfig);
    }

    void execute(OpenScene&& request)
    {
        loadScene(request.contextId, request.path);
    }

    void execute(SetProperty&& request)
    {
        World& world = services.worlds.get(contexts().get(request.contextId).world);
        if (!world.isValid(request.entity))
            return;

        if (!world.hasComponent(request.entity, request.componentType))
            return;

        auto component = world.editComponent(request.entity, request.componentType);
        request.property->set(component, request.value);
    }

    void execute(SetCameraMouseLookEnabled&& request)
    {
        EditorCamera::setActive(request.window, request.enabled);
    }
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
    Engine::addSystem(HierarchySystem::callbacks);
    Engine::addSystem(TransformSystem::callbacks);
    Engine::addSystem(EntityProxySystem::callbacks);
    Engine::addSystem(BoundingBoxSystem::callbacks);
    Engine::addSystem(RenderSynchronizer::callbacks);

    Engine::init();

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
            execute(OpenProject{.contextId = defaultContextId, .path = config.lastProject});
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

void Editor::rebuildPanelView()
{
    panelView.clear();

    for (auto& panel : panels)
        panelView.push_back(panel.get());
}

Entity Editor::ensureCamera(World& world)
{
    auto hasCamera = [&world](Entity entity) { return world.hasComponent<CameraComponent>(entity); };
    auto entities = world.getEntitiesRange();
    if (auto cameraEntityIt = std::ranges::find_if(entities, hasCamera); cameraEntityIt != entities.end())
    {
        return *cameraEntityIt;
    }

    const Entity camera = world.createEntity();
    world.addComponent<CameraComponent>(camera, CameraComponent{.fov = 60.f});
    world.addComponent<NameComponent>(camera, "Main Camera");

    constexpr Vec3 position{2.f, 2.f, 2.f};
    const Vec3 direction{Math::normalize(-position)};
    const Quat rotation{Math::rotation(forwardVector(), direction)};

    world.addComponent<TransformComponent>(camera,{
                                               .position = position,
                                               .rotation = rotation
                                           });
    return camera;
}

void Editor::loadScene(EditingContextId contextId, const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path))
        return;

    services.scenes.loadScene(contexts().get(contextId).world, path);
}
