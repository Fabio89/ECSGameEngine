module;

#include "EngineExport.h"

export module Input;
import Math;
import Window;

export using ::KeyCode;

export namespace Input
{
    ENGINE_API
    Vec2 getCursorScreenPosition(WindowHandle window);

    ENGINE_API
    bool isKeyDown(KeyCode key);

    ENGINE_API
    bool isKeyJustPressed(KeyCode key);

    ENGINE_API
    bool isKeyJustReleased(KeyCode key);

    ENGINE_API
    void setCursorMode(WindowHandle window, CursorMode mode);

    ENGINE_API
    void setCursorType(WindowHandle window, CursorType type);

    void init(WindowHandle window);
    void postUpdate();
}
