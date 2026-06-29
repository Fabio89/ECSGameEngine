module;
#include "EngineExport.h"
export module EditorUIContext;
import World;
import Window;

export struct ENGINE_API EditorUIContext
{
    World* world{};
    WindowHandle window{};
};
