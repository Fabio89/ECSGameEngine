export module System.Camera;
import Component.Camera;
import Component.Transform;
import Math;
import System;
import std;

export class System_Camera final : public System
{
    void onUpdate(World& world, Player& player, [[maybe_unused]] float deltaTime) override
    {
        const float aspectRatio = world.getRenderManager().getAspectRatio();

        Entity cameraEntity = player.getMainCamera();
        if (cameraEntity != invalidId())
        {
            check(std::ranges::contains(world.getComponentTypesInEntity(cameraEntity), CameraComponent::typeId()), "Main camera doesn't have a camera component...??");
            const TransformComponent& transform = world.readComponent<TransformComponent>(cameraEntity);
            CameraComponent& camera = world.editComponent<CameraComponent>(cameraEntity);

            camera.projectionMatrix = Math::perspective( Math::radians(camera.fov), aspectRatio, camera.nearPlane, camera.farPlane);
            camera.projectionMatrix[1][1] *= -1.0f;

            const Vec3 forward = Math::rotate(transform.rotation, forwardVector());
            camera.viewMatrix = Math::lookAt(transform.position, transform.position + forward, upVector());

            world.getRenderManager().setCamera({camera.viewMatrix, camera.projectionMatrix});
        }
    }
};