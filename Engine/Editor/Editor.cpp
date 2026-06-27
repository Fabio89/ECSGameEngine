module Editor;
import Component.Hierarchy;
import Component.Transform;
import Editor.Camera;
import Editor.Components;
import Editor.Gizmos;
import Editor.Events;
import Editor.ImGui;
import Editor.Panel.Hierarchy;
import Editor.Panel.Inspector;
import Editor.Panel.MainMenu;
import Engine;
import Input;
import Math;
import Physics;
import World;
import World.Events;

enum class EditMode : UInt8
{
    Translate = 0,
    Rotate = 1,
    Scale = 2,
    None = 3
};

namespace Editor
{
    EventSubscription subscription;

    void draw();
    bool isMouseAvailable();
    bool isKeyboardAvailable();
}

namespace
{
    EditMode currentEditMode{EditMode::None};
    std::array<Entity, 3> gizmos;
    std::unordered_map<Entity, Entity> entitiesToBoundingBoxGizmos;
    Entity selectedGizmoAxis;
    Editor::Selection currentSelection; // TODO(refactoring): transition to this from single Entity selection
    std::optional<Vec3> projectedCursorPositionLastFrame;
    std::vector<std::unique_ptr<Panel>> panels;
}

constexpr Entity& getGizmo(EditMode editMode)
{
    return gizmos[static_cast<UInt8>(editMode)];
}

void attachToSelection(World& world, Entity gizmo)
{
    // TODO(refactoring): Gizmos probably shouldn't attach to entities as children
    Entity firstSelected = currentSelection.isEmpty() ? Entity{} : currentSelection.get().front();
    if (!check(firstSelected.isValid(), "Tried to attach a gizmo to an invalid entity!"))
        return;

    HierarchyUtils::setParent(world, gizmo, firstSelected);
    world.editComponent<TransformComponent>(gizmo).scale = 0.2f / world.readComponent<TransformComponent>(firstSelected).scale;
}

void setEditMode(World& world, EditMode editMode)
{
    if (editMode != currentEditMode)
    {
        currentEditMode = editMode;
        for (Entity gizmo : gizmos)
        {
            check(world.isValid(gizmo), "Gizmo entity is invalid. Please ensure all gizmos are properly initialized before setting edit mode.", ErrorType::Error);
            const bool shouldShow = !currentSelection.isEmpty() && editMode != EditMode::None && gizmo == getGizmo(editMode);
            EditorUtils::setGizmoVisible(world, gizmo, shouldShow);
            if (shouldShow)
            {
                attachToSelection(world, gizmo);
            }
        }
    }
}

void Editor::Selection::add(Entity entity)
{
    if (!contains(entity))
        m_entities.push_back(entity);
}

void Editor::Selection::remove(Entity entity)
{
    std::erase(m_entities, entity);
}

void Editor::Selection::clear()
{
    m_entities.clear();
}

bool Editor::Selection::contains(Entity entity) const
{
    return std::ranges::find(m_entities, entity) != m_entities.end();
}

bool Editor::Selection::isEmpty() const
{
    return m_entities.empty();
}

void Editor::Selection::set(std::span<const Entity> entities)
{
    auto validEntities = entities | std::views::filter([](Entity e){ return e.isValid(); });
    m_entities.assign(validEntities.begin(), validEntities.end());

    Engine::events().publish(SelectionChangedEvent{.selection = m_entities});
}

void Editor::Selection::setSingle(Entity entity)
{
    m_entities.clear();
    if (entity.isValid())
        m_entities.push_back(entity);

    Engine::events().publish(SelectionChangedEvent{.selection = m_entities});
}

std::span<const Entity> Editor::Selection::get() const
{
    return m_entities;
}

void Editor::init(EditorContext context)
{
    EditorComponents::init();
    editorContext = context;
    addPanel<Panels::Hierarchy>();
    addPanel<Panels::Inspector>();
    addPanel<Panels::MainMenu>();

    Engine::setEditorDrawCallback(draw);

    subscription += Engine::events().subscribe([](const SceneLoadedEvent&)
    {
        entitiesToBoundingBoxGizmos.clear();
        selectedGizmoAxis = {};
        currentSelection.clear();
        createGizmos();
    });
}

void Editor::shutdown()
{
    subscription = {};
}

void Editor::createGizmos()
{
    getGizmo(EditMode::Translate) = EditorUtils::createTranslationGizmo(*editorContext.world);
    getGizmo(EditMode::Rotate) = EditorUtils::createRotationGizmo(*editorContext.world);
    getGizmo(EditMode::Scale) = EditorUtils::createScaleGizmo(*editorContext.world);
}

void Editor::update(float deltaTime)
{
    World& world = *editorContext.world;
    if (isKeyboardAvailable())
    {
        if (Input::isKeyJustPressed(KeyCode::Q))
        {
            setEditMode(world, EditMode::None);
        } else if (Input::isKeyJustPressed(KeyCode::W))
        {
            setEditMode(world, EditMode::Translate);
        } else if (Input::isKeyJustPressed(KeyCode::E))
        {
            setEditMode(world, EditMode::Rotate);
        } else if (Input::isKeyJustPressed(KeyCode::R))
        {
            setEditMode(world, EditMode::Scale);
        }
    }

    if (isMouseAvailable())
    {
        const Vec2 cursorPosition = Input::getCursorPosition(editorContext.window);
        const Ray ray = Physics::rayFromScreenPosition(world, Engine::getPlayer(), cursorPosition);

        Entity firstSelectedEntity = currentSelection.isEmpty() ? Entity{} : currentSelection.get().front();
        if (Engine::isValid(firstSelectedEntity) && Engine::isValid(selectedGizmoAxis) && Input::isKeyDown(KeyCode::MouseButtonLeft))
        {
            TransformComponent& transform = world.editComponent<TransformComponent>(firstSelectedEntity);

            const Vec3 gizmoAxisDirection = [&]
            {
                const Entity gizmoEntity = HierarchyUtils::getParent(world, selectedGizmoAxis);
                const GizmoComponent& gizmo = world.readComponent<GizmoComponent>(gizmoEntity);
                if (selectedGizmoAxis == gizmo.xAxisEntity)
                    return TransformUtils::right(transform);
                if (selectedGizmoAxis == gizmo.yAxisEntity)
                    return TransformUtils::up(transform);
                if (selectedGizmoAxis == gizmo.zAxisEntity)
                    return TransformUtils::forward(transform);
                report("Invalid gizmo axis entity");
                return Vec3{};
            }();

            const Plane movePlane
            {
                .point = transform.position,
                .normal = Math::cross(
                    TransformUtils::right(world.readComponent<TransformComponent>(Engine::getPlayer().getMainCamera())),
                    gizmoAxisDirection)
            };

            const std::optional<Vec3> projectedCursorPosition = Physics::intersectRayPlane(ray, movePlane);

            if (projectedCursorPositionLastFrame.has_value() && projectedCursorPosition.has_value())
            {
                const auto delta = Math::dot(*projectedCursorPosition - *projectedCursorPositionLastFrame,
                                             gizmoAxisDirection);
                transform.position += gizmoAxisDirection * delta;
                projectedCursorPositionLastFrame = *projectedCursorPosition;
            }
            projectedCursorPositionLastFrame = projectedCursorPosition;
        } else
        {
            selectedGizmoAxis = {};
            projectedCursorPositionLastFrame = {};
        }

        if (Input::isKeyJustPressed(KeyCode::MouseButtonLeft))
        {
            selectedGizmoAxis = Physics::lineTrace(world, ray, TraceChannelFlags::Gizmo);

            const Entity hitEntity = Physics::lineTrace(world, ray, TraceChannelFlags::Default);

            setSingleSelection(hitEntity);
        }
    }

    EditorCamera::setActive(editorContext.window, Input::isKeyDown(KeyCode::MouseButtonRight));
    EditorCamera::update(editorContext.window, world, Engine::getPlayer(), deltaTime);
}

void Editor::setSingleSelection(Entity entity)
{
    if (currentSelection.contains(entity) && currentSelection.get().size() == 1)
        return;

    World& world = *editorContext.world;
    for (Entity selected : currentSelection.get())
    {
        EditorUtils::setGizmoVisible(world, entitiesToBoundingBoxGizmos.at(selected), false);
    }

    currentSelection.setSingle(entity);

    if (entity.isValid())
    {
        if (currentEditMode != EditMode::None)
        {
            const Entity gizmo = getGizmo(currentEditMode);
            attachToSelection(world, gizmo);
            EditorUtils::setGizmoVisible(world, gizmo, true);
        }

        auto it = entitiesToBoundingBoxGizmos.find(entity);
        if (it == entitiesToBoundingBoxGizmos.end())
            it = entitiesToBoundingBoxGizmos.emplace(
                entity, EditorUtils::createBoundingBoxGizmo(world, entity)).first;

        const Entity boundingBoxGizmo = it->second;
        EditorUtils::setGizmoVisible(world, boundingBoxGizmo, true);
    } else
    {
        if (currentEditMode != EditMode::None)
        {
            EditorUtils::setGizmoVisible(world, getGizmo(currentEditMode), false);
        }
    }
}

void Editor::setSelection(std::span<const Entity> entities)
{
    // TODO(feature): Multi-selection
    if (!entities.empty())
        setSingleSelection(entities.front());
}

Editor::Selection& Editor::selection()
{
    return currentSelection;
}

std::span<const Entity> Editor::getSelection()
{
    return currentSelection.get();
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

    for (auto& panel : panels)
    {
        panel->draw();
    }
}

bool Editor::isMouseAvailable()
{
    return !ImGui::GetCurrentContext() || !ImGui::GetIO().WantCaptureMouse;
}

bool Editor::isKeyboardAvailable()
{
    return !ImGui::GetCurrentContext() || !ImGui::GetIO().WantCaptureKeyboard;
}