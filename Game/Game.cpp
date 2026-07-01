import Editor;
import Engine;
import std;

void onUpdate(float deltaTime);

void waitTillClosed()
{
    bool running{true};
    while (running)
    {
        //std::this_thread::sleep_for(Engine::getDeltaTime());
        running = Engine::update();
        if (!running)
            return;
        Editor::update();
    }
}

int main()
{
    Engine::init();
    Editor::init(Engine::getEditorContext());
    Engine::start();

    // openProject("C:/Users/march/Documents/Mashi Projects/MashiTestProject/project.ma");
    Engine::openProject("/home/Fabio/Projects/MashiTestProject/project.ma");

    Entity camera = Engine::createEntity();
    Engine::addComponent<NameComponent>(camera, "TestCamera");
    Engine::addComponent<TransformComponent>(camera);
    Engine::addComponent<TagsComponent>(camera, {.tags = {"Uno", "Due", "Tre", "Quattro", "Cinque", "Sei"}});
    Engine::addComponent<CameraComponent>(camera);

    Engine::printArchetypeStatus();

    waitTillClosed();
    Editor::shutdown();
    Engine::shutdown();
    return 0;
}
