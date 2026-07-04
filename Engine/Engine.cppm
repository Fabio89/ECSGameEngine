module;

#include "EngineExport.h"

export module Engine;
export import Core;
export import EngineComponents;
export import EventBus;
import ComponentRegistry;
import EditorUIContext;
import FileSystem;
import Geometry;
import Input;
import Log;
import Math;
import Physics;
import Player;
import Render.EditorCallbacks;
import Render.RenderManager;
import Window;
import World;

namespace Engine
{
    RenderManager renderManager;
    World world{{&renderManager}};
}

export namespace Engine
{
    //------------------------------------------------------------------------------------------------------------------------
    // Application
    //------------------------------------------------------------------------------------------------------------------------

    using ::WindowMode;
    using ::WindowCreateInfo;

    ENGINE_API void init();

    ENGINE_API void start();

    ENGINE_API bool update();

    ENGINE_API float getSimulationDeltaTime();

    ENGINE_API float getRenderDeltaTime();

    ENGINE_API void shutdown();

    //------------------------------------------------------------------------------------------------------------------------
    // Input
    //------------------------------------------------------------------------------------------------------------------------

    ENGINE_API Entity getEntityUnderCursor();

    //------------------------------------------------------------------------------------------------------------------------
    // Scene
    //------------------------------------------------------------------------------------------------------------------------

    ENGINE_API void openScene(const std::filesystem::path& path);

    //------------------------------------------------------------------------------------------------------------------------
    // ECS
    //------------------------------------------------------------------------------------------------------------------------

    ENGINE_API Entity createEntity();

    ENGINE_API void removeEntity(Entity entity);

    ENGINE_API bool isValid(Entity entity);

    template <ValidComponentData T, typename... Args>
    ENGINE_API T& addComponent(Entity entity, Args&&... args);

    template <ValidComponentData T>
    ENGINE_API T& addComponent(Entity entity, T&& args);

    template <ValidComponentData T>
    ENGINE_API bool hasComponent(Entity entity);

    ENGINE_API bool hasComponent(Entity entity, TypeId componentTypeId);

    template <ValidComponentData T>
    ENGINE_API const T& readComponent(Entity entity);

    ENGINE_API const ComponentBase& readComponent(Entity entity, TypeId componentType);

    template <ValidComponentData T>
    ENGINE_API T& editComponent(Entity entity);

    ENGINE_API ComponentBase& editComponent(Entity entity, TypeId componentType);

    ENGINE_API auto getEntitiesRange() { return world.getEntitiesRange(); }

    template <ValidComponentData First, ValidComponentData ... Rest>
    ENGINE_API std::generator<std::tuple<Entity, const First&, const Rest&...>> view();

    template <ValidComponentData First, ValidComponentData ... Rest>
    ENGINE_API std::generator<std::tuple<Entity, First&, Rest&...>> view();

    //------------------------------------------------------------------------------------------------------------------------
    // Events
    //------------------------------------------------------------------------------------------------------------------------

    ENGINE_API EventBus& events();

    //------------------------------------------------------------------------------------------------------------------------
    // Editor Integration
    //------------------------------------------------------------------------------------------------------------------------

    ENGINE_API EditorUIContext getEditorContext();

    ENGINE_API void setEditorCallbacks(EditorCallbacks callbacks);

    ENGINE_API void setViewportArea(Rect area);

    ENGINE_API Ray getViewportCursorRay(const World& world);

    //------------------------------------------------------------------------------------------------------------------------
    // DEBUG -TEMPORARY
    //------------------------------------------------------------------------------------------------------------------------

    ENGINE_API Player& getPlayer();

    ENGINE_API void printArchetypeStatus();
}

//------------------------------------------------------------------------------------------------------------------------
// Template Definitions
//------------------------------------------------------------------------------------------------------------------------
namespace Engine
{
    template<ValidComponentData T, typename... Args>
    T& addComponent(Entity entity, Args&&... args) { return world.addComponent<T>(entity, std::forward<Args>(args)...); }

    template <ValidComponentData T>
    T& addComponent(Entity entity, T&& args) { return world.addComponent<T>(entity, std::forward<T>(args)); }

    template <ValidComponentData T>
    bool hasComponent(Entity entity) { return world.hasComponent<T>(entity); }

    bool hasComponent(Entity entity, TypeId componentTypeId);

    template <ValidComponentData T>
    const T& readComponent(Entity entity) { return world.readComponent<T>(entity); }

    const ComponentBase& readComponent(Entity entity, TypeId componentType);

    template <ValidComponentData T>
    T& editComponent(Entity entity) { return world.editComponent<T>(entity); }

    ComponentBase& editComponent(Entity entity, TypeId componentType);

    template <ValidComponentData First, ValidComponentData ... Rest>
    std::generator<std::tuple<Entity, const First&, const Rest&...>> view() { return world.view<First, Rest...>(); }

    template <ValidComponentData First, ValidComponentData ... Rest>
    std::generator<std::tuple<Entity, First&, Rest&...>> view() { return world.view<First, Rest...>(); }
}
