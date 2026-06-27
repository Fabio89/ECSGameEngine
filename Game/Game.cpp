import Editor;
import Engine;
import std;

void onUpdate(float deltaTime);

bool performLoops(int count)
{
    bool running{true};
    constexpr std::chrono::milliseconds deltaTime{8};
    for (int i = 0; i < count && running; ++i)
    {
        constexpr float deltaTimeMs = static_cast<float>(deltaTime.count()) * 0.001f;
        running = Engine::update(deltaTimeMs);
        if (!running)
            return false;
        Editor::update(deltaTimeMs);
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
        constexpr float deltaTimeMs = static_cast<float>(deltaTime.count()) * 0.001f;
        onUpdate(deltaTimeMs);
        running = Engine::update(deltaTimeMs);
        if (!running)
            return;
        Editor::update(deltaTimeMs);
    }
}

void onUpdate(float deltaTime)
{
}

int main()
{
    Engine::init({2560, 1440});
    Editor::init(Engine::getEditorContext());

    // openProject("C:/Users/march/Documents/Mashi Projects/MashiTestProject/project.ma");
    Engine::openProject("/home/Fabio/Projects/MashiTestProject/project.ma");

    Entity camera = Engine::createEntity();
    Engine::addComponent<NameComponent>(camera, "TestCamera");
    Engine::addComponent<TransformComponent>(camera);
    Engine::addComponent<TagsComponent>(camera, {.tags = {"Uno", "Due", "Tre", "Quattro", "Cinque", "Sei"}});
    Engine::addComponent<CameraComponent>(camera);

    Engine::printArchetypeStatus();
    
    waitTillClosed();
    Engine::shutdown();
    return 0;
}
