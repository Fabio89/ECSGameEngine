module Component.Camera;
import Component.Transform;
import Engine;

Camera CameraUtils::toRenderCamera(const World& world, ViewportId viewportId, Entity cameraEntity)
{
    if (!cameraEntity.isValid())
        return {};

    const float aspectRatio = Engine::getViewportAspectRatio(viewportId);

    const TransformComponent& transform = world.readComponent<TransformComponent>(cameraEntity);
    const CameraComponent& camera = world.readComponent<CameraComponent>(cameraEntity);

    Mat4 projectionMatrix = Math::perspective(Math::radians(camera.fov), aspectRatio, camera.nearPlane, camera.farPlane);
    projectionMatrix[1][1] *= -1.0f;

    const Vec3 forward = Math::rotate(transform.rotation, forwardVector());
    const Mat4 viewMatrix = Math::lookAt(transform.position, transform.position + forward, upVector());

    return {.view = viewMatrix, .proj = projectionMatrix};
}
