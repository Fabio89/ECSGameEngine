export module Editor.Services;
import Engine.WorldManager;
import EventBus;
import Render.CommandProcessor;

export struct EditorServices
{
    WorldManager& worlds;
    EventBus& events;
    RenderCommandQueue& renderCommands;
};