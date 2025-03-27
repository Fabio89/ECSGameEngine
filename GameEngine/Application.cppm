export module Application;
import ComponentRegistry;
import Core;
import Input;
import Math;
import Player;
import World;
import Render.IRenderManager;
import Wrapper.Glfw;
import Wrapper.Windows;

export using ::GLFWwindow;
export using ::HWND;

//------------------------------------------------------------------------------------------------------------------------
// Application
//------------------------------------------------------------------------------------------------------------------------

extern "C" __declspec(dllexport)
inline int getCoolestNumber() { return 22; }

export extern "C" __declspec(dllexport)
void runTest(GLFWwindow* window = nullptr);

export extern "C" __declspec(dllexport)
GLFWwindow* createWindow(HWND parentHwnd, int width, int height);

export extern "C" __declspec(dllexport)
void engineInit(GLFWwindow* window);

export extern "C" __declspec(dllexport)
bool engineUpdate(GLFWwindow* window, float deltaTime);

export extern "C" __declspec(dllexport)
void engineShutdown(GLFWwindow* window);

export extern "C" __declspec(dllexport)
void setViewport(GLFWwindow* window, int x, int y, int width, int height);

//------------------------------------------------------------------------------------------------------------------------
// Input
//------------------------------------------------------------------------------------------------------------------------
export extern "C" __declspec(dllexport)
void addKeyEventCallback(Input::KeyEventCallback callback);

export extern "C" __declspec(dllexport)
Entity getEntityUnderCursor(GLFWwindow* window);

export __declspec(dllexport)
Vec2 getCursorPosition(GLFWwindow* window);

//------------------------------------------------------------------------------------------------------------------------
// Project
//------------------------------------------------------------------------------------------------------------------------

export extern "C" __declspec(dllexport)
void openProject(const char* path);

export extern "C" __declspec(dllexport)
void saveCurrentProject();

export extern "C" __declspec(dllexport)
void serializeScene(char* buffer, int bufferSize);

export extern "C" __declspec(dllexport)
void patchEntity(Entity entity, const char* json);

//------------------------------------------------------------------------------------------------------------------------
// DEBUG -TEMPORARY
//------------------------------------------------------------------------------------------------------------------------
export extern "C" __declspec(dllexport)
void updateDebugCamera(GLFWwindow* window, float deltaTime);

export __declspec(dllexport)
World& getWorld();

export __declspec(dllexport)
void printArchetypeStatus();

export __declspec(dllexport)
Player& getPlayer();

export __declspec(dllexport)
ComponentBase& editComponent(Entity entity, ComponentTypeId typeId);

export template<ValidComponent T>
T& editComponent(Entity entity)
{
    return reinterpret_cast<T&>(editComponent(entity, T::typeId()));
}