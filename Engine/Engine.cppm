module;

#include "EngineExport.h"

export module Engine;
export import ComponentRegistry;
export import Core;
export import EngineComponents;
export import Glfw;
export import Input;
export import Log;
export import Math;
export import Physics;
export import Player;
export import Project;
export import World;
import Render.RenderManager;

export namespace Engine
{
    //------------------------------------------------------------------------------------------------------------------------
    // Application
    //------------------------------------------------------------------------------------------------------------------------

    enum class ENGINE_API WindowMode
    {
        Standalone,
        Embedded
    };

    struct ENGINE_API WindowCreateInfo
    {
        int width = 1280;
        int height = 720;
        WindowMode mode = WindowMode::Standalone;
    };

    ENGINE_API GLFWwindow* createWindow(const WindowCreateInfo& info);
    ENGINE_API void engineInit(GLFWwindow* window);
    ENGINE_API bool engineUpdate(GLFWwindow* window, float deltaTime);
    ENGINE_API void engineShutdown(GLFWwindow* window);
    ENGINE_API void setViewport(GLFWwindow* window, int x, int y, int width, int height);

    //------------------------------------------------------------------------------------------------------------------------
    // Input
    //------------------------------------------------------------------------------------------------------------------------

    ENGINE_API Entity getEntityUnderCursor(GLFWwindow* window);

    //------------------------------------------------------------------------------------------------------------------------
    // Project
    //------------------------------------------------------------------------------------------------------------------------

    ENGINE_API void openProject(const char* path);
    ENGINE_API void startEmptyProject();
    ENGINE_API void saveCurrentProject();
    ENGINE_API void serializeScene(char* buffer, int bufferSize);
    ENGINE_API void patchEntity(Entity entity, const char* json);

    //------------------------------------------------------------------------------------------------------------------------
    // DEBUG -TEMPORARY
    //------------------------------------------------------------------------------------------------------------------------

    ENGINE_API World& getWorld();

    ENGINE_API void printArchetypeStatus();

    ENGINE_API Player& getPlayer();

    ENGINE_API ComponentBase& editComponent(Entity entity, ComponentTypeId typeId);

    template<ValidComponentData T>
    T& editComponent(Entity entity)
    {
        return reinterpret_cast<T&>(editComponent(entity, Component<T>::typeId()));
    }
}