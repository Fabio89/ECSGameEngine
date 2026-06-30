module Editor.Panels.Viewport;
import Editor;
import Editor.Requests;
import Editor.TransformTool;
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

Rect calculateViewportArea(ViewportMode mode)
{
    const ImGui::ImVec2 panelPosition = ImGui::GetCursorScreenPos();
    const ImGui::ImVec2 availableSize = ImGui::GetContentRegionAvail();

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

Panels::ViewportPanel::ViewportPanel(const PanelCreateInfo& info)
    : PanelImpl{info},
      m_controller{Editor::addController<ViewportController>(info.contextId)}
{
    m_controller.get().tools().createTools();

    m_sub += Engine::events().subscribe([this, contextId = info.contextId](const Engine::SceneLoadedEvent&)
    {
        m_controller.get().tools().createTools();
    });
}

void Panels::ViewportPanel::doDraw()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImGui::ImVec2{0, 0});
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::ImVec4{0, 0, 0, 0});
    ImGui::Begin("Viewport");

    const Rect viewportArea = calculateViewportArea(ViewportMode::Editor);
    Engine::setViewportArea(viewportArea);

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
        const ImGui::ImVec2 mouse = ImGui::GetMousePos();

        const Vec2 uv
        {
            (mouse.x - viewportArea.position.x) / viewportArea.size.width,
            (mouse.y - viewportArea.position.y) / viewportArea.size.height
        };

        const Ray ray = Physics::rayFromViewportUV(context().world, Engine::getPlayer(), uv);

        const Entity hitEntity = Physics::lineTrace(context().world, ray, TraceChannelFlags::Default);

        Editor::request(Editor::ChangeSelection{.contextId = context().id, .entities = {hitEntity}});
    }

    const bool enableCameraControls = ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Right);
    EditorCamera::setActive(getWindow(),  enableCameraControls);

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void Panels::ViewportPanel::setCurrentTool(EntityEditingMode type)
{
    m_controller.get().tools().setCurrentTool(type);
}

ViewportController::ViewportController(EditingContext& context)
    : EditorController{context},
      m_tools{context},
      m_selectionGizmos{context}
{
}

void ViewportController::update(float dt)
{
    EditorController::update(dt);
    m_tools.update();
}
