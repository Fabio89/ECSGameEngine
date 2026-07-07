module;

#include "EngineExport.h"

export module Engine;
export import Core;
export import EngineComponents;
export import EventBus;
import ComponentRegistry;
import FileSystem;
import Geometry;
import Input;
import Log;
import Math;
import Physics;
import Player;
import Render.EditorCallbacks;
import Render.RenderManager;
import Window;
import World;
import WorldHandle;

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

    //------------------------------------------------------------------------------------------------------------------------
    // Events
    //------------------------------------------------------------------------------------------------------------------------

    ENGINE_API EventBus& events();

    //------------------------------------------------------------------------------------------------------------------------
    // Editor Integration
    //------------------------------------------------------------------------------------------------------------------------

    ENGINE_API void setEditorCallbacks(EditorCallbacks callbacks);

    ENGINE_API void setViewportArea(Rect area);

    ENGINE_API Ray getViewportCursorRay(const World& world);

    //------------------------------------------------------------------------------------------------------------------------
    // DEBUG -TEMPORARY
    //------------------------------------------------------------------------------------------------------------------------

    ENGINE_API Player& getPlayer();
}
