export module EngineSystems;
export import System.BoundingBox;
export import System.Camera;
export import System.Hierarchy;
export import System.LineRender;
export import System.Model;
export import System.Transform;
import System;

namespace EngineSystems
{
    export void init(World& world);
    export void update(World& world, Player& player, float deltaTime);
    export void reset();
    export void shutdown();

    void addSystem(World& world, std::unique_ptr<System> system);

    template <typename T>
    void addSystem(World& world);

    template <typename T, typename... TSystems>
    void addSystems(World& world);

    std::vector<std::unique_ptr<System>> systems;
}

export void EngineSystems::init(World& world)
{
    addSystems<
        BoundingBoxSystem,
        CameraSystem,
        HierarchySystem,
        LineRenderSystem,
        ModelSystem,
        TransformSystem
    >(world);
}

void EngineSystems::update(World& world, Player& player, float deltaTime)
{
    for (auto& system : systems)
        system->update(world, player, deltaTime);
}

void EngineSystems::reset()
{
}

void EngineSystems::shutdown()
{
    systems.clear();
}

void EngineSystems::addSystem(World& world, std::unique_ptr<System> system)
{
    System* systemPtr = systems.emplace_back(std::move(system)).get();
    world.registerSystem({
        .onComponentAdded = [&world, systemPtr](Entity entity, TypeId componentId)
        {
            systemPtr->notifyComponentAdded(world, entity, componentId);
        },
        .onEntityRemoved = [&world, systemPtr](Entity entity)
        {
            systemPtr->notifyEntityDestroyed(world, entity);
        }
    });
}

template<typename T>
void EngineSystems::addSystem(World& world)
{
    addSystem(world, std::make_unique<T>());
}

template <typename T, typename... TSystems>
void EngineSystems::addSystems(World& world)
{
    addSystem<T>(world);

    if constexpr (sizeof...(TSystems) > 0)
    {
        addSystems<TSystems...>(world);
    }
}