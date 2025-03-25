import Engine;
import std;
import <chrono>;

::GLFWwindow* window{};

void onUpdate(float deltaTime);

bool performLoops(int count)
{
    bool running{true};
    constexpr std::chrono::milliseconds deltaTime{8};
    for (int i = 0; i < count && running; ++i)
    {
        running = engineUpdate(window, static_cast<float>(deltaTime.count()) * 0.001f);
        if (!running)
            return false;
        //std::this_thread::sleep_for(deltaTime);
    }
    return true;
}

void waitTillClosed()
{
    bool running{true};
    constexpr std::chrono::milliseconds deltaTime{8};
    while (running)
    {
        std::this_thread::sleep_for(deltaTime);
        onUpdate(static_cast<float>(deltaTime.count()) * 0.001f);
        running = engineUpdate(window, static_cast<float>(deltaTime.count()) * 0.001f);
        if (!running)
            return;
    }
}

Vec2 lastCursorPosition;

void onUpdate(float deltaTime)
{
    float speed = 3.0f * deltaTime;
    float rotationSpeed = 10000.0f * deltaTime;
    
    float xMovement = (static_cast<int>(Input::isKeyDown(KeyCode::D)) - static_cast<int>(Input::isKeyDown(KeyCode::A))) * speed;
    float yMovement = (static_cast<int>(Input::isKeyDown(KeyCode::E)) - static_cast<int>(Input::isKeyDown(KeyCode::Q))) * speed;
    float zMovement = (static_cast<int>(Input::isKeyDown(KeyCode::W)) - static_cast<int>(Input::isKeyDown(KeyCode::S))) * speed;

    float dYaw = 0;
    float dPitch = 0;

    if (Input::isKeyDown(KeyCode::MouseButtonRight))
    {
        Input::setCursorMode(window, CursorMode::Disabled);
        const Vec2 cursorPosition = Input::getCursorPosition(window);
        if (cursorPosition.x != lastCursorPosition.x)
        {
            dYaw = rotationSpeed * deltaTime * (cursorPosition.x - lastCursorPosition.x );
            dPitch = rotationSpeed * deltaTime * (cursorPosition.y - lastCursorPosition.y);
        }
    }
    else
    {
        Input::setCursorMode(window, CursorMode::Normal);
    }
    lastCursorPosition = Input::getCursorPosition(window);

    //if (xMovement != 0 || yMovement != 0 || zMovement != 0)
    {
        auto cameraEntity = getPlayer().getMainCamera();
        
        auto& transform = editComponent2<TransformComponent>(cameraEntity);
        const Vec3 forward = TransformUtils::forward(transform);
        const Vec3 right = TransformUtils::right(transform);
        const Vec3 up = TransformUtils::up(transform);
        
        transform.position += right * xMovement + up * yMovement + forward * zMovement;

        Quat yawQuat = Math::angleAxis(dYaw, Vec3(0.0f, 1.0f, 0.0f));
        Quat pitchQuat = Math::angleAxis(dPitch, right);

        Quat combinedQuat = Math::normalize(yawQuat * pitchQuat);

        // Apply the combined rotation to the transform orientation
        transform.rotation = Math::normalize(combinedQuat * transform.rotation);
        
        //log(std::format("Camera position: {}, {}, {}", transform.position.x, transform.position.y, transform.position.z));
    }
}

int main()
{
    window = createWindow(nullptr, 800, 600);
    engineInit(window);

    openProject("C:/Users/march/Documents/Mashi Projects/TestProject/project.ma");
    
    // char buf[4096];
    // serializeScene(buf, sizeof(buf));
    // std::cout << buf;


    waitTillClosed();
    engineShutdown(window);
    return 0;
}
