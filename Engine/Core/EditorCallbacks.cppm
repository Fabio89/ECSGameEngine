module;
#include "EngineExport.h"
export module Render.EditorCallbacks;
import std;

export struct ENGINE_API EditorCallbacks
{
    std::function<void()> imguiInit;
    std::function<void()> draw;
};