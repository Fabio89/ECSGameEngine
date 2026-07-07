module Editor;
import AssetManager;
import Editor.Camera;
import Editor.Components;
import Editor.Controller;
import Editor.Events;
import Editor.ImGuiUI;
import Editor.Panels.Hierarchy;
import Editor.Panels.Details;
import Editor.Panels.MainMenu;
import Editor.Panels.Viewport;
import Editor.Project;
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
    std::vector<Panel*> panelView;
}

namespace Editor
{
    void rebuildPanelView();
    Entity ensureCamera(World& world);
    void loadScene(EditingContextId contextId, const std::filesystem::path& path);

    void execute(ChangeSelection&& request)
    {
        contexts().get(request.contextId).selection.set(request.entities);
    }

    void execute(OpenProject&& request)
    {
        const ProjectConfig config = loadProjectConfig(request.path / "project.toml");

        AssetManager::setContentRoot(request.path / config.contentRoot);
        AssetManager::loadDatabase(request.path / config.assetDatabase);

        loadScene(request.contextId, request.path / config.startupScene);
    }

    void execute(OpenScene&& request)
    {
        loadScene(request.contextId, request.path);
    }

    void execute(SetProperty&& request)
    {
        World& world = Engine::getWorld(contexts().get(request.contextId).world);
        if (!world.isValid(request.entity))
            return;

        if (!world.hasComponent(request.entity, request.componentType))
            return;

        request.property->set(&world.editComponent(request.entity, request.componentType), request.value);
    }
}

void Editor::addPanel(std::unique_ptr<Panel> panel)
{
    panels.emplace_back(std::move(panel));
    rebuildPanelView();
}

void Editor::init()
{
    Engine::init();

    EditorComponents::init();

    editorContext = {.world = Engine::createWorld(), .window = Engine::getWindow()};

    //WorldHandle world2 = Engine::createWorld();

    initPropertyDrawers();

    const EditingContextId defaultContextId = contexts().add(editorContext.world);
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

    subscription += Engine::events().subscribe([](const Engine::SceneLoadedEvent& event)
    {
        for (EditingContext& context : contexts().getAll())
        {
            if (&context.world == &event.world)
            {
                context.selection.clear();
            }
        }
    });

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
    if (!Engine::update())
        return false;

    EditorRequest request;

    while (requests.tryPop(request))
        std::visit([]<typename Request>(Request&& r) { execute(std::forward<Request>(r)); }, std::move(request));

    for (auto& [contextId, controllerManager] : controllerManagers)
        controllerManager.update(Engine::getSimulationDeltaTime(), contexts().get(contextId).snapshotPublisher);

    EditorCamera::update(editorContext.window, Engine::getWorld(editorContext.world), Engine::getPlayer(), Engine::getSimulationDeltaTime());

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
    world.addComponent<TransformComponent>(camera);

    auto& transform = world.editComponent<TransformComponent>(camera);
    transform.position = {2.f, 2.f, 2.f};
    const Vec3 dir = Math::normalize(-transform.position);
    const Quat rot = Math::rotation(forwardVector(), dir);
    transform.rotation = rot;

    return camera;
}

void Editor::loadScene(EditingContextId contextId, const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path))
        return;

    World& world = Engine::getWorld(contexts().get(contextId).world);
    world.loadScene(path);
    Engine::getPlayer().setMainCamera(world, ensureCamera(world));
}
