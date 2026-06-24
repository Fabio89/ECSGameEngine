import Engine;
import ComponentArray;
import Editor;
import Glfw;
import std;

::GLFWwindow* window{};

void onUpdate(float deltaTime);

bool performLoops(int count)
{
    bool running{true};
    constexpr std::chrono::milliseconds deltaTime{8};
    for (int i = 0; i < count && running; ++i)
    {
        running = Engine::engineUpdate(window, static_cast<float>(deltaTime.count()) * 0.001f);
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
        running = Engine::engineUpdate(window, static_cast<float>(deltaTime.count()) * 0.001f);
        if (!running)
            return;
    }
}

void onUpdate(float deltaTime)
{
    Editor::update(window, deltaTime);
}

ComponentArray<TransformComponent> transforms;

int main()
{
    window = Engine::createWindow({2560, 1440});
    Engine::engineInit(window);
    // openProject("C:/Users/march/Documents/Mashi Projects/MashiTestProject/project.ma");
    Engine::openProject("/home/Fabio/Projects/MashiTestProject/project.ma");
    //startEmptyProject();
    Editor::init();

    Entity camera = Engine::getWorld().createEntity();
    Engine::getWorld().addComponent<NameComponent>(camera, "TestCamera");
    Engine::getWorld().addComponent<TransformComponent>(camera);
    Engine::getWorld().addComponent<TagsComponent>(camera, {.tags = {"Uno", "Due", "Tre", "Quattro", "Cinque", "Sei"}});
    Engine::getWorld().addComponent<CameraComponent>(camera);

    // char buf[4096];
    // serializeScene(buf, sizeof(buf));
    // std::cout << buf;

    std::vector<Entity> entities;
    for (Entity entity : Engine::getWorld().getEntitiesRange())
    {
        entities.push_back(entity);
    }
    for (Entity entity : entities)
    {
        
    }

    Engine::printArchetypeStatus();
    
    waitTillClosed();
    Engine::engineShutdown(window);
    return 0;
}
