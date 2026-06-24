import Engine;
import ComponentArray;
import EditorBridge;
import std;
import Wrapper.Glfw;

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
    editorUpdate(window, deltaTime);
}

ComponentArray<TransformComponent> transforms;

int main()
{
    window = createWindow({2560, 1440});
    engineInit(window);
    openProject("C:/Users/march/Documents/Mashi Projects/MashiTestProject/project.ma");
    //openProject("/home/Fabio/Projects/MashiTestProject/project.ma");
    //startEmptyProject();
    editorInit();

    Entity camera = getWorld().createEntity();
    getWorld().addComponent<NameComponent>(camera, "TestCamera");
    getWorld().addComponent<TransformComponent>(camera);
    getWorld().addComponent<TagsComponent>(camera, {.tags = {"Uno", "Due", "Tre", "Quattro", "Cinque", "Sei"}});
    getWorld().addComponent<CameraComponent>(camera);

    // char buf[4096];
    // serializeScene(buf, sizeof(buf));
    // std::cout << buf;

    std::vector<Entity> entities;
    for (Entity entity : getWorld().getEntitiesRange())
    {
        entities.push_back(entity);
    }
    for (Entity entity : entities)
    {
        
    }

    printArchetypeStatus();
    
    waitTillClosed();
    engineShutdown(window);
    return 0;
}
