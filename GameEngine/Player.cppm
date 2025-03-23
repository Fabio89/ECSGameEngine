export module Engine:Player;
import :Ecs;
import :World;
import :Component.Camera;
import :Component.Transform;

export class Player
{
public:
    void setMainCamera(const World& world, Entity camera)
    {
        if (std::ranges::contains(world.getComponentTypesInEntity(camera), CameraComponent::typeId))
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