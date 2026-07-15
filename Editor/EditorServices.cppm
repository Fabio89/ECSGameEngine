export module Editor.Services;
import Engine.WorldManager;
import EventBus;
import Render.CommandProcessor;
import Render.Viewport;
import SceneManager;

export struct EditorServices
{
    WorldManager& worlds;
    ViewportManager& viewports;
    SceneManager& scenes;
    EventBus& events;
    RenderCommandQueue& renderCommands;
};

export class EditorServiceConsumer
{
public:
    explicit EditorServiceConsumer(EditorServices& services) : m_services{services} {}

protected:
    EditorServices& services() { return m_services; }
    const EditorServices& services() const { return m_services; }

private:
    EditorServices& m_services;
};
