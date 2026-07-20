module Editor.Panels.Viewport;
import Components.Gizmo;
import Editor.Camera;
import Editor.Events;
import Editor.Requests;
import Editor;
import Engine.Camera;
import Engine;
import Geometry;
import Input;
import Math;
import Physics;
import Render.CommandProcessor;
import Render.Commands;
import World.Events;
import World;

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
    const Quat rotation = Math::angleAxis(Math::radians(-135.f), Vec3{0, 1, 0})
                          * Math::angleAxis(Math::radians(33.f), Vec3{1, 0, 0});

    world.addComponent<TransformComponent>(camera, {
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

ViewportController::ViewportController(EditorServices& services, EditingContext& context, SharedMailbox mailbox, WindowHandle window)
    : EditorControllerImpl{services, context, std::move(mailbox)},
      m_tools{TransformToolContext{.services = services, .editing = context, .viewportId = m_viewportId, .window = m_window}},
      m_selectionGizmos{services, context},
      m_camera{ensureCamera(services.worlds.get(context.editorWorld))},
      m_window{window}
{
}

void ViewportController::update(float dt, Editor::SnapshotFrame& frame)
{
    EditorControllerImpl::update(dt, frame);
    m_tools.update();

    if (m_viewportId.isValid())
    {
        EditorCamera::update(m_window, services().worlds.get(context().editorWorld), m_camera, Engine::getSimulationDeltaTime());
        const Camera camera = CameraUtils::toRenderCamera(services().worlds.get(context().editorWorld), m_viewportId, m_camera);
        services().viewports.setCamera(m_viewportId, camera);
        services().renderCommands.addCommand(RenderCommands::SetCamera{.world = context().world, .camera = camera});
        services().renderCommands.addCommand(RenderCommands::SetCamera{.world = context().editorWorld, .camera = camera});
    }
}

void ViewportController::execute(Requests::AssignViewport&& request)
{
    m_viewportId = request.viewportId;
}

void ViewportController::execute(Requests::SetCameraMouseLookEnabled&& request)
{
    EditorCamera::setActive(m_window, request.enabled);
}

void ViewportController::execute(Requests::HandleMouseSelect&& request)
{
    if (!m_viewportId.isValid())
        return;

    const Ray ray = Engine::getViewportCursorRay(m_viewportId);

    const World& editorWorld = services().worlds.get(context().editorWorld);

    const Entity hitGizmoHandle = Physics::lineTrace(editorWorld, ray, TraceChannelFlags::Gizmo);
    if (!editorWorld.isValid(hitGizmoHandle) || !editorWorld.hasComponent<GizmoHandleComponent>(hitGizmoHandle))
    {
        const World& mainWorld = services().worlds.get(context().world);
        const Entity hitEntity = Physics::lineTrace(mainWorld, ray, TraceChannelFlags::Default);
        if (hitEntity.isValid())
            context().selection.setSingle(hitEntity);
        else
            context().selection.clear();
    }
}

void ViewportController::execute(Requests::SetEditingMode&& request)
{
    m_tools.setCurrentTool(request.mode);
}

ViewportSnapshot ViewportController::buildSnapshot(const EditingContext& context)
{
    return {
         .selectionEnabled = m_tools.isSelectionEnabled()
    };
}

Panels::ViewportPanel::ViewportPanel(const PanelCreateInfo& info)
    : PanelImpl{info}
{
    createController(getWindow());
}

void Panels::ViewportPanel::doDraw()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{0, 0, 0, 0});
    ImGui::Begin(Name);

    const Rect viewportArea = calculateViewportArea(ViewportMode::Editor);

    if (!m_viewportId)
    {
        m_viewportId = Engine::createViewport({context().world, context().editorWorld}, viewportArea);
        request(Requests::AssignViewport{m_viewportId});
    }
    else
        Engine::setViewportArea(m_viewportId, viewportArea);

    drawFpsCounter();

    if (!ImGui::IsMouseDown(ImGuiMouseButton_Right))
    {
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
    }

    auto setMouseLook = [this](bool enabled)
    {
        request(Requests::SetCameraMouseLookEnabled{enabled});
    };

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
        setMouseLook(false);
    else if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        setMouseLook(true);

    if (const Snapshot* snapshot = getSnapshot(); ImGui::IsWindowHovered() && snapshot)
    {
        if (snapshot->selectionEnabled && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            request(Requests::HandleMouseSelect{});
    }

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void Panels::ViewportPanel::setCurrentTool(EntityEditingMode type)
{
    request(Requests::SetEditingMode{type});
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
