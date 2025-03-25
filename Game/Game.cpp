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


void onUpdate(float deltaTime)
{
    float speed = 3.0f * deltaTime;
    float xMovement = (static_cast<int>(Input::isKeyDown(KeyCode::D)) - static_cast<int>(Input::isKeyDown(KeyCode::A))) * speed;
    float yMovement = (static_cast<int>(Input::isKeyDown(KeyCode::E)) - static_cast<int>(Input::isKeyDown(KeyCode::Q))) * speed;
    float zMovement = (static_cast<int>(Input::isKeyDown(KeyCode::W)) - static_cast<int>(Input::isKeyDown(KeyCode::S))) * speed;

    if (xMovement != 0 || yMovement != 0 || zMovement != 0)
    {
        auto cameraEntity = getPlayer().getMainCamera();
        
        auto& transform = editComponent2<TransformComponent>(cameraEntity);
        // const Vec3 forward = TransformUtils::forward(transform);
        // const Vec3 right = TransformUtils::right(transform);
        // const Vec3 up = TransformUtils::up(transform);

        Vec3 euler = Math::eulerAngles(transform.rotation); // Converts quaternion to pitch (X), yaw (Y), roll (Z)

        float pitch = Math::degrees(Math::pitch(transform.rotation));
        float yaw = Math::degrees(Math::yaw(transform.rotation));
        float roll = Math::degrees(Math::roll(transform.rotation));  

        log(std::format("Yaw: {}, Pitch: {}, Roll: {}", yaw, pitch, roll));
        float pitchRadians = Math::radians(pitch);
        float yawRadians = Math::radians(yaw);
        
        // Forward vector (Z forward in your system)
        Vec3 forward = Math::normalize(Vec3(
            cos(pitchRadians) * sin(yawRadians), // X component (negative for left-handed yaw)
            -sin(pitchRadians),                   // Y component
            cos(pitchRadians) * cos(yawRadians)  // Z component
        ));

        
        // Right vector (perpendicular to forward, X right in your system)
        Vec3 right = Math::normalize(Math::cross(Vec3(0.0f, 1.0f, 0.0f), forward));
        
        // Up vector (always Y up in your system)
        Vec3 up = -Math::normalize(Math::cross(right, forward));


        transform.position += right * xMovement + up * yMovement + forward * zMovement;

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
