module;

#include "EngineExport.h"

export module Editor;
import Core;
import Window;
import World;

export namespace Editor
{
    ENGINE_API void init(World& world);

    ENGINE_API void createGizmos(World& world);

    ENGINE_API void update(World& world, WindowHandle window, float deltaTime);
}
