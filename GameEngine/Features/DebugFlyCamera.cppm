export module Feature.DebugFlyCamera;
import Component.Transform;
import Input;
import Math;
import Player;
import World;
import Wrapper.Glfw;

float locationSmoothingSpeed = 200.0f;
float rotationSmoothingSpeed = 20.0f;
float delayBeforeDrag = 0.1f;

export namespace DebugCamera
{
    void update(GLFWwindow* window, World& world, const Player& player, float deltaTime);
}
