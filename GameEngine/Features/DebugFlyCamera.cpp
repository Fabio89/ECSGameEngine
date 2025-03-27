module Feature.DebugFlyCamera;
import Core;

Vec2 lastCursorPosition;

void updateCameraTransform(GLFWwindow* window, World& world, Entity camera, float deltaTime, bool allowRotation)
{
    float speed = 3.0f * deltaTime;
    float xMovement = (static_cast<int>(Input::isKeyDown(KeyCode::D)) - static_cast<int>(Input::isKeyDown(KeyCode::A))) * speed;
    float yMovement = (static_cast<int>(Input::isKeyDown(KeyCode::E)) - static_cast<int>(Input::isKeyDown(KeyCode::Q))) * speed;
    float zMovement = (static_cast<int>(Input::isKeyDown(KeyCode::W)) - static_cast<int>(Input::isKeyDown(KeyCode::S))) * speed;
    
    auto& transform = world.editComponent<TransformComponent>(camera);
    const Vec3 forward = TransformUtils::forward(transform);
    const Vec3 right = TransformUtils::right(transform);
    const Vec3 up = TransformUtils::up(transform);

    const Vec3 targetPosition = transform.position + right * xMovement + up * yMovement + forward * zMovement;
    transform.position = Math::lerp(transform.position, targetPosition, locationSmoothingSpeed * deltaTime);
    
    if (allowRotation)
    {
        float rotationMultiplier = 300.0f;
        float maxRotSpeed = 300.f * deltaTime;
        float dYaw = 0;
        float dPitch = 0;

        const Vec2 cursorPosition = Input::getCursorPosition(window);
        dYaw = Math::clamp(rotationMultiplier * deltaTime * (cursorPosition.x - lastCursorPosition.x), -maxRotSpeed, maxRotSpeed);
        dPitch = Math::clamp(rotationMultiplier * deltaTime * (cursorPosition.y - lastCursorPosition.y), -maxRotSpeed, maxRotSpeed);

        Quat yawQuat = Math::angleAxis(dYaw, Vec3(0.0f, 1.0f, 0.0f));
        Quat pitchQuat = Math::angleAxis(dPitch, right);

        Quat targetRotation = Math::normalize(yawQuat * pitchQuat * transform.rotation);
        //transform.rotation = Math::normalize(Math::slerp(transform.rotation, targetRotation, rotationSmoothingSpeed * deltaTime));
        transform.rotation = targetRotation;
    }
}

float rotationCooldown = delayBeforeDrag;

void DebugCamera::update(GLFWwindow* window, World& world, const Player& player, float deltaTime)
{
    if (Input::isKeyDown(KeyCode::MouseButtonRight))
    {
        Input::setCursorMode(window, CursorMode::Disabled);
        rotationCooldown = Math::max(rotationCooldown - deltaTime, 0.f);

        if (auto cameraEntity = player.getMainCamera(); cameraEntity != invalidId())
            updateCameraTransform(window, world, cameraEntity, deltaTime, rotationCooldown == 0.f);
    }
    else
    {
        rotationCooldown = delayBeforeDrag;
        Input::setCursorMode(window, CursorMode::Normal);
    }

    lastCursorPosition = Input::getCursorPosition(window);
}
