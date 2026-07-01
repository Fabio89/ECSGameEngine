export module Editor.Camera;
import Component.Transform;
import Input;
import Math;
import Player;
import Window;
import World;

export namespace EditorCamera
{
    void setActive(WindowHandle window, bool active);
    void update(WindowHandle window, World& world, const Player& player, float deltaTime);
}
