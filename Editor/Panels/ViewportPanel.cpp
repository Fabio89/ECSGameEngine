module Editor.Panels.Viewport;
import Editor;
import Editor.Events;
import Editor.Requests;
import Engine;
import Geometry;
import Math;
import Physics;
import Editor.Camera;
import World;
import World.Events;
import Render.Commands;
import Engine.Camera;
import Render.CommandProcessor;

enum class ViewportMode
{
    Editor,
    GameCamera
};

namespace
{
    float smoothedGameDeltaTime = 0.016f;
    float smoothedRenderDeltaTime = 0.016f;
}

Entity ensureCamera(World& world)
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

Rect calculateViewportArea(ViewportMode mode)
{
    const ImVec2 panelPosition = ImGui::GetCursorScreenPos();
    const ImVec2 availableSize = ImGui::GetContentRegionAvail();

    switch (mode)
    {
        case ViewportMode::Editor:
        default:
        {
            return {
                IVec2{panelPosition.x, panelPosition.y},
                Size2D{static_cast<Int32>(availableSize.x), static_cast<Int32>(availableSize.y)}
            };
        }
        case ViewportMode::GameCamera:
        {
            constexpr float sceneAspect = 16.0f / 9.0f;

            float panelWidth = availableSize.x;
            float panelHeight = availableSize.y;

            float panelAspect = panelWidth / panelHeight;

            Size2D renderSize;

            if (panelAspect > sceneAspect)
            {
                // Too wide: pillarbox
                renderSize.height = panelHeight;
                renderSize.width = panelHeight * sceneAspect;
            }
            else
            {
                // Too tall: letterbox
                renderSize.width = panelWidth;
                renderSize.height = panelWidth / sceneAspect;
            }

            IVec2 offset
            {
                (panelWidth - renderSize.width) / 2 + panelPosition.x,
                (panelHeight - renderSize.height) / 2 + panelPosition.y
            };

            return {offset, renderSize};
        }
    }
}

ViewportController::ViewportController(EditorServices& services, EditingContext& context, ViewportId viewportId)
    : EditorControllerImpl{services, context},
      m_tools{TransformToolContext{.services = services, .editing = context, .viewportId = m_id}},
      m_selectionGizmos{services, context},
      m_camera{ensureCamera(services.worlds.get(context.editorWorld))},
      m_id{viewportId}
{
    m_tools.createTools();

    m_subscription += services.events.subscribe([this, contextId = context.id](const EditorEvents::EntityEditingModeChanged& event)
    {
        if (event.contextId == contextId)
            m_tools.setCurrentTool(event.mode);
    });

    m_subscription += services.worlds.subscribe([this, &worlds = services.worlds, worldHandle = context.world](const WorldEvents::SceneLoaded& event)
    {
        if (worlds.get(event.world).getHandle() == worldHandle)
            m_tools.createTools();
    });
}

void ViewportController::update(float dt, Editor::SnapshotFrame& frame)
{
    EditorControllerImpl::update(dt, frame);
    m_tools.update();

    EditorCamera::update(Engine::getWindow(), services().worlds.get(context().editorWorld), m_camera, Engine::getSimulationDeltaTime());
    const Camera camera = CameraUtils::toRenderCamera(services().worlds.get(context().editorWorld), m_id, m_camera);
    services().viewports.setCamera(m_id, camera);
    services().renderCommands.addCommand(RenderCommands::SetCamera{.world = context().world, .camera = camera});
}

ViewportSnapshot ViewportController::buildSnapshot(const EditingContext& context)
{
    return {};
}

Panels::ViewportPanel::ViewportPanel(const PanelCreateInfo& info)
    : PanelImpl{info}
{
}

void Panels::ViewportPanel::doDraw()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{0, 0, 0, 0});
    ImGui::Begin(Name);

    const Rect viewportArea = calculateViewportArea(ViewportMode::Editor);

    if (!m_id)
    {
        m_id = Engine::createViewport({context().world, context().editorWorld}, viewportArea);
        Editor::addController<ViewportController>(context().id, m_id);
    }
    else
        Engine::setViewportArea(m_id, viewportArea);

    drawFpsCounter();

    if (ImGui::IsKeyPressed(ImGuiKey_Q))
    {
        setCurrentTool(EntityEditingMode::None);
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_W))
    {
        setCurrentTool(EntityEditingMode::Translate);
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_E))
    {
        setCurrentTool(EntityEditingMode::Rotate);
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_R))
    {
        setCurrentTool(EntityEditingMode::Scale);
    }

    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        Editor::request(Editor::SelectEntityUnderCursor{.contextId = context().id, .window = getWindow(), .viewport = m_id});
    }

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
        Editor::request(Editor::SetCameraMouseLookEnabled{getWindow(), false});
    else if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        Editor::request(Editor::SetCameraMouseLookEnabled{getWindow(), true});

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void Panels::ViewportPanel::setCurrentTool(EntityEditingMode type)
{
    Editor::request(Editor::SetEntityEditingMode{.contextId = context().id, .mode = type});
}

void Panels::ViewportPanel::drawFpsCounter() const
{
    constexpr float smoothing = 5.0f;

    const ImVec2 min = ImGui::GetCursorScreenPos();
    const ImVec2 max = {min.x + ImGui::GetContentRegionAvail().x, min.y + ImGui::GetContentRegionAvail().y};

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    static constexpr ImColor color{255,255,255};
    ImVec2 pos = {max.x - 200.0f, min.y + 10.0f};

    const float labelWidth = ImGui::CalcTextSize("Simulation:").x;

    auto getFpsString = [](float dt) { return std::format("{:.0f} FPS ({:.1f} ms)", 1 / dt, dt * 1000); };

    const float dt = ImGui::GetIO().DeltaTime;
    const float t = 1.0f - std::exp(-smoothing * dt);

    smoothedGameDeltaTime = Math::lerp(smoothedGameDeltaTime, Engine::getSimulationDeltaTime(), t);
    drawList->AddText(pos, color, "Simulation:");
    drawList->AddText({pos.x + labelWidth + 10.0f, pos.y}, color, getFpsString(smoothedGameDeltaTime).c_str());

    pos.y += ImGui::GetTextLineHeight();

    smoothedRenderDeltaTime = Math::lerp(smoothedRenderDeltaTime, Engine::getRenderDeltaTime(), t);
    drawList->AddText(pos, color, "Render:");
    drawList->AddText({pos.x + labelWidth + 10.0f, pos.y}, color, getFpsString(smoothedRenderDeltaTime).c_str());
}
