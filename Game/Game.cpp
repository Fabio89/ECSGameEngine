import Engine;
import ComponentArray;
import Editor;
import std;

void onUpdate(float deltaTime);

bool performLoops(int count)
{
    bool running{true};
    constexpr std::chrono::milliseconds deltaTime{8};
    for (int i = 0; i < count && running; ++i)
    {
        running = Engine::update(static_cast<float>(deltaTime.count()) * 0.001f);
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
        running = Engine::update(static_cast<float>(deltaTime.count()) * 0.001f);
        if (!running)
            return;
    }
}

void onUpdate(float deltaTime)
{
}

int main()
{
    Engine::init({2560, 1440});
    // openProject("C:/Users/march/Documents/Mashi Projects/MashiTestProject/project.ma");
    Engine::openProject("/home/Fabio/Projects/MashiTestProject/project.ma");

    Entity camera = Engine::createEntity();
    Engine::addComponent<NameComponent>(camera, "TestCamera");
    Engine::addComponent<TransformComponent>(camera);
    Engine::addComponent<TagsComponent>(camera, {.tags = {"Uno", "Due", "Tre", "Quattro", "Cinque", "Sei"}});
    Engine::addComponent<CameraComponent>(camera);

    // char buf[4096];
    // serializeScene(buf, sizeof(buf));
    // std::cout << buf;

    std::vector<Entity> entities;
    for (Entity entity : Engine::getEntitiesRange())
    {
        entities.push_back(entity);
    }
    for (Entity entity : entities)
    {
        
    }

    Engine::printArchetypeStatus();
    
    waitTillClosed();
    Engine::shutdown();
    return 0;
}
