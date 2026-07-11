module;

#include "EngineExport.h"

export module Engine;
export import Core;
export import EngineComponents;
import ComponentRegistry;
import Engine.SystemManager;
import Engine.Viewport;
import Engine.WorldManager;
import Geometry;
import Log;
import Math;
import Physics;
import Render.CommandProcessor;
import Render.EditorCallbacks;
import Window;
import World;

export namespace Engine
{
    //------------------------------------------------------------------------------------------------------------------------
    // Application
    //------------------------------------------------------------------------------------------------------------------------

    using ::WindowMode;
    using ::WindowCreateInfo;

    ENGINE_API void init();

    ENGINE_API void start();

    ENGINE_API bool update();

    ENGINE_API float getSimulationDeltaTime();

    ENGINE_API float getRenderDeltaTime();

    ENGINE_API void shutdown();

    WindowHandle getWindow();

    //------------------------------------------------------------------------------------------------------------------------
    // ECS
    //------------------------------------------------------------------------------------------------------------------------

    ENGINE_API WorldHandle createWorld();

    ENGINE_API World& getWorld(WorldHandle handle);

    ENGINE_API WorldManager& worlds();

    ENGINE_API void addSystem(SystemCallbacks callbacks);

    //------------------------------------------------------------------------------------------------------------------------
    // Editor Integration
    //------------------------------------------------------------------------------------------------------------------------

    ENGINE_API ViewportId createViewport(WorldHandle world, Rect area);

    ENGINE_API void setEditorCallbacks(EditorCallbacks callbacks);

    ENGINE_API void setViewportArea(ViewportId id, Rect area);

    ENGINE_API Ray getViewportCursorRay(const World& world);

    //------------------------------------------------------------------------------------------------------------------------
    // Rendering
    //------------------------------------------------------------------------------------------------------------------------

    ENGINE_API RenderCommandQueue& getRenderCommandQueue();

    //------------------------------------------------------------------------------------------------------------------------
    // DEBUG -TEMPORARY
    //------------------------------------------------------------------------------------------------------------------------

    ENGINE_API float getViewportAspectRatio();
}
