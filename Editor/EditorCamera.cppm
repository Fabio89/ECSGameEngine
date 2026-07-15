export module Editor.Camera;
import Components.Transform;
import Input;
import Math;
import Window;
import World;

export namespace EditorCamera
{
    void setActive(WindowHandle window, bool active);
    void update(WindowHandle window, World& world, Entity camera, float deltaTime);
}
