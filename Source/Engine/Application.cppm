module;

#include "EngineExport.h"

export module Application;
import ComponentRegistry;
import Core;
import Input;
import Math;
import Player;
import World;
import Render.RenderManager;
import Wrapper.Glfw;

//------------------------------------------------------------------------------------------------------------------------
// Application
//------------------------------------------------------------------------------------------------------------------------

extern "C" ENGINE_API
inline int getCoolestNumber() { return 22; }

export extern "C" ENGINE_API
void runTest(GLFWwindow* window = nullptr);

export
enum class ENGINE_API WindowMode
{
    Standalone,
    Embedded
};

export
struct ENGINE_API WindowCreateInfo
{
    int width = 1280;
    int height = 720;
    WindowMode mode = WindowMode::Standalone;
};

export ENGINE_API
GLFWwindow* createWindow(const WindowCreateInfo& info);

export extern "C" ENGINE_API
void engineInit(GLFWwindow* window);

export extern "C" ENGINE_API
bool engineUpdate(GLFWwindow* window, float deltaTime);

export extern "C" ENGINE_API
void engineShutdown(GLFWwindow* window);

export extern "C" ENGINE_API
void setViewport(GLFWwindow* window, int x, int y, int width, int height);

//------------------------------------------------------------------------------------------------------------------------
// Input
//------------------------------------------------------------------------------------------------------------------------
export extern "C" ENGINE_API
void addKeyEventCallback(Input::KeyEventCallback callback);

export extern "C" ENGINE_API
Entity getEntityUnderCursor(GLFWwindow* window);

export ENGINE_API
Vec2 getCursorPosition(GLFWwindow* window);

//------------------------------------------------------------------------------------------------------------------------
// Project
//------------------------------------------------------------------------------------------------------------------------

export extern "C" ENGINE_API
void openProject(const char* path);

export extern "C" ENGINE_API
void startEmptyProject();

export extern "C" ENGINE_API
void saveCurrentProject();

export extern "C" ENGINE_API
void serializeScene(char* buffer, int bufferSize);

export extern "C" ENGINE_API
void patchEntity(Entity entity, const char* json);

//------------------------------------------------------------------------------------------------------------------------
// DEBUG -TEMPORARY
//------------------------------------------------------------------------------------------------------------------------
export ENGINE_API
World& getWorld();

export ENGINE_API
void printArchetypeStatus();

export ENGINE_API
Player& getPlayer();

export ENGINE_API
ComponentBase& editComponent(Entity entity, ComponentTypeId typeId);

export template<ValidComponentData T>
T& editComponent(Entity entity)
{
    return reinterpret_cast<T&>(editComponent(entity, Component<T>::typeId()));
}