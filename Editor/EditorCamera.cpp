module Editor.Camera;
import Core;

namespace
{
    bool isActive{};
    float delayBeforeDrag = 0.1f;
    float movementSpeed = 10.f;
}

void updateCameraTransform(WindowHandle window, World& world, Entity camera, float deltaTime, bool allowRotation)
{
    const Vec3 movement
    {
        (static_cast<int>(Input::isKeyDown(KeyCode::D)) - static_cast<int>(Input::isKeyDown(KeyCode::A))),
        (static_cast<int>(Input::isKeyDown(KeyCode::E)) - static_cast<int>(Input::isKeyDown(KeyCode::Q))),
        (static_cast<int>(Input::isKeyDown(KeyCode::W)) - static_cast<int>(Input::isKeyDown(KeyCode::S)))
    };

    auto& transform = world.editComponent<TransformComponent>(camera);
    const Vec3 forward = TransformUtils::forward(transform);
    const Vec3 right = TransformUtils::right(transform);
    const Vec3 up = TransformUtils::up(transform);

    Vec3 velocity = (right * movement.x + up * movement.y + forward * movement.z) * movementSpeed;
    //cameraVelocity = Math::lerp(cameraVelocity, targetVelocity, 1.0f - std::exp(-acceleration * deltaTime));
    transform.position += velocity * deltaTime;

    if (allowRotation)
    {
        float rotationMultiplier = 0.0015f;
        float dYaw = 0;
        float dPitch = 0;

        const Vec2 cursorDelta = Input::getCursorDelta(window);
        dYaw = rotationMultiplier * cursorDelta.x;
        dPitch = rotationMultiplier * cursorDelta.y;

        Quat yawQuat = Math::angleAxis(dYaw, Vec3(0,1,0));
        transform.rotation = yawQuat * transform.rotation;
        Vec3 newRight = TransformUtils::right(transform);
        Quat pitchQuat = Math::angleAxis(dPitch, newRight);
        transform.rotation = Math::normalize(pitchQuat * transform.rotation);
    }
}

float rotationCooldown = delayBeforeDrag;

void EditorCamera::setActive(WindowHandle window, bool active)
{
    if (active != isActive)
    {
        isActive = active;
        if (active)
        {
            Input::setCursorMode(window, CursorMode::Disabled);
        }
        else
        {
            rotationCooldown = delayBeforeDrag;
            Input::setCursorMode(window, CursorMode::Normal);
        }
    }
}

void EditorCamera::update(WindowHandle window, World& world, float deltaTime)
{
    if (!isActive)
        return;
    
    rotationCooldown = Math::max(rotationCooldown - deltaTime, 0.f);

    if (auto cameraEntity = world.getActiveCamera(); world.isValid(cameraEntity))
        updateCameraTransform(window, world, cameraEntity, deltaTime, rotationCooldown == 0.f);
}
