export module Editor.Camera;
import Component.Transform;
import Input;
import Math;
import Window;
import World;

export namespace EditorCamera
{
    void setActive(WindowHandle window, bool active);
    void update(WindowHandle window, World& world, float deltaTime);
}
