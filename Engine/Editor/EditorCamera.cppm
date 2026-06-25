export module Editor.Camera;
import Component.Transform;
import Input;
import Math;
import Player;
import Window;
import World;

float locationSmoothingSpeed = 200.0f;
float rotationSmoothingSpeed = 20.0f;
float delayBeforeDrag = 0.1f;

export namespace EditorCamera
{
    void setActive(WindowHandle window, bool active);
    void update(WindowHandle window, World& world, const Player& player, float deltaTime);
}
