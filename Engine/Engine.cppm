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
import Render.Viewport;
import SceneManager;
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

    UInt64 getCurrentFrame();

    //------------------------------------------------------------------------------------------------------------------------
    // ECS
    //------------------------------------------------------------------------------------------------------------------------

    ENGINE_API WorldHandle createWorld();

    ENGINE_API World& getWorld(WorldHandle handle);

    ENGINE_API WorldManager& worlds();

    ENGINE_API void addSystem(SystemCallbacks callbacks);

    //------------------------------------------------------------------------------------------------------------------------
    // Scene
    //------------------------------------------------------------------------------------------------------------------------

    ENGINE_API SceneManager& scenes();

    //------------------------------------------------------------------------------------------------------------------------
    // Editor Integration
    //------------------------------------------------------------------------------------------------------------------------

    ENGINE_API ViewportId createViewport(std::vector<WorldHandle> worlds, Rect area);

    ENGINE_API void setEditorCallbacks(EditorCallbacks callbacks);

    ENGINE_API void setViewportArea(ViewportId id, Rect area);

    ENGINE_API Rect getViewportArea(ViewportId id);

    ENGINE_API Ray getViewportCursorRay(ViewportId id);

    ENGINE_API ViewportManager& viewports();

    ENGINE_API float getViewportAspectRatio(ViewportId id);

    //------------------------------------------------------------------------------------------------------------------------
    // Rendering
    //------------------------------------------------------------------------------------------------------------------------

    ENGINE_API RenderCommandQueue& getRenderCommandQueue();

    //------------------------------------------------------------------------------------------------------------------------
    // DEBUG -TEMPORARY
    //------------------------------------------------------------------------------------------------------------------------

}
