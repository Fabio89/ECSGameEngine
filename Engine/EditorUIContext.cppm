module;
#include "EngineExport.h"
export module EditorUIContext;
import Window;
import WorldHandle;

export struct ENGINE_API EditorUIContext
{
    WorldHandle world;
    WindowHandle window;
};
