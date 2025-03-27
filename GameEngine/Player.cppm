export module Player;
import Component.Camera;
import Component.Transform;
import Ecs;
import World;
import std;

export class Player
{
public:
    void setMainCamera(const World& world, Entity camera)
    {
        if (world.hasComponent<CameraComponent>(camera))
        {
            m_mainCamera = camera;
            log(std::format("Assigned camera entity '{}' as main camera", camera));
        }
    }

    Entity getMainCamera() const
    {
        return m_mainCamera;
    }
    
private:
    Entity m_mainCamera{invalidId()};
};