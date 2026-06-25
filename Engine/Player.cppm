export module Player;
import Component.Camera;
import Component.Transform;
import Core;
import World;

export class Player
{
public:
    void setMainCamera(const World& world, Entity camera)
    {
        if (camera == invalidId() || world.hasComponent<CameraComponent>(camera))
        {
            m_mainCamera = camera;
            log(std::format("Assigned camera entity '{}' as main camera", camera));
        }
    }

    [[nodiscard]]
    Entity getMainCamera() const
    {
        return m_mainCamera;
    }
    
private:
    Entity m_mainCamera{invalidId()};
};