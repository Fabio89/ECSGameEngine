module Editor.Panels.Viewport;
import Editor;
import Editor.Requests;
import Engine;
import Geometry;
import Math;
import Physics;
import Editor.Camera;
import World;
import World.Events;

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

ViewportController::ViewportController(EditingContext& context)
    : EditorControllerImpl{context},
      m_tools{context},
      m_selectionGizmos{context}
{
}

void ViewportController::update(float dt, Editor::SnapshotFrame& frame)
{
    EditorControllerImpl::update(dt, frame);
    m_tools.update();

}

void ViewportController::selectEntityUnderCursor() {}

ViewportSnapshot ViewportController::buildSnapshot(const EditingContext& context)
{
    return {};
}

Panels::ViewportPanel::ViewportPanel(const PanelCreateInfo& info)
    : PanelImpl{info},
      m_controller{Editor::addController<ViewportController>(info.contextId)}
{
    m_controller.get().tools().createTools();

    m_sub += Engine::events().subscribe([this](const Engine::SceneLoadedEvent&)
    {
        m_controller.get().tools().createTools();
    });
}

void Panels::ViewportPanel::doDraw()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{0, 0, 0, 0});
    ImGui::Begin(Name);

    const Rect viewportArea = calculateViewportArea(ViewportMode::Editor);

    if (!m_id)
        m_id = Engine::createViewport(context().world, viewportArea);
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
        const ImVec2 mouse = ImGui::GetMousePos();

        const Vec2 uv
        {
            (mouse.x - viewportArea.position.x) / viewportArea.size.width,
            (mouse.y - viewportArea.position.y) / viewportArea.size.height
        };

        const World& world = Engine::getWorld(context().world);
        const Ray ray = Physics::rayFromViewportUV(world, Engine::getPlayer(), uv);
        const Entity hitEntity = Physics::lineTrace(world, ray, TraceChannelFlags::Default);

        Editor::request(Editor::ChangeSelection{.contextId = context().id, .entities = {hitEntity}});
    }

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
        EditorCamera::setActive(getWindow(), false);
    else if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        EditorCamera::setActive(getWindow(), true);

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void Panels::ViewportPanel::setCurrentTool(EntityEditingMode type)
{
    m_controller.get().tools().setCurrentTool(type);
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
