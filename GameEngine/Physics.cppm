export module Physics;
import Math;
import Player;
import World;

export
struct Ray
{
    Vec3 origin;
    Vec3 direction;
};

namespace Physics
{
    export __declspec(dllexport)
    Entity lineTrace(const World& world, const Ray& ray);

    export __declspec(dllexport)
    Ray rayFromScreenPosition(const World& world, const Player& player, Vec2 screenPosition);
}
