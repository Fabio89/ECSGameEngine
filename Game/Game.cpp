import Editor;
import Engine;
import std;

void onUpdate(float deltaTime);

int main()
{
    Editor::run();

    // openProject("C:/Users/march/Documents/Mashi Projects/MashiTestProject/project.ma");
    //Editor::openProject("/home/Fabio/Projects/MashiTestProject");

    // Entity camera = Engine::createEntity();
    // Engine::addComponent<NameComponent>(camera, "TestCamera");
    // Engine::addComponent<TransformComponent>(camera);
    // Engine::addComponent<TagsComponent>(camera, {.tags = {"Uno", "Due", "Tre", "Quattro", "Cinque", "Sei"}});
    // Engine::addComponent<CameraComponent>(camera);

    // Engine::printArchetypeStatus();
    //
    // waitTillClosed();
    // Editor::shutdown();
    // Engine::shutdown();
    return 0;
}
