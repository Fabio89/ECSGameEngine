module;
#include "EngineExport.h"
export module EditorContext;
import World;
import Window;

export struct ENGINE_API EditorContext
{
    World* world{};
    WindowHandle window{};
};