import Engine;
import std;
import <chrono>;

::GLFWwindow* window{};

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

int main()
{
    window = createWindow(nullptr, 800, 600);
    engineInit(window);
    
    openProject("C:/Users/march/Documents/Mashi Projects/TestProject/project.ma");

    performLoops(5);



    // performLoops(10);
    // openProject("C:/Users/march/Documents/Mashi Projects/EmptyProject/project.ma");
    // performLoops(10);
    // openProject("C:/Users/march/Documents/Mashi Projects/TestProject/project.ma");
    // performLoops(10);
    // openProject("C:/Users/march/Documents/Mashi Projects/EmptyProject/project.ma");
    
    performLoops(5);
    engineShutdown(window);
    return 0;
}