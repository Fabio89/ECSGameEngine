export module EngineSystems;
export import System;
export import System.BoundingBox;
export import System.Camera;
export import System.Model;
export import System.Transform;
import std;

namespace EngineSystems
{
    export void init(World& world);
    export void update(World& world, Player& player, float deltaTime);
    export void reset();
    
    void addSystem(World& world, std::unique_ptr<System> system);

    template <typename T>
    void addSystem(World& world);

    template <typename T, typename... T_Systems>
    void addSystems(World& world);

    std::vector<std::unique_ptr<System>> systems;
}

export void EngineSystems::init(World& world)
{
    addSystems<
        System_BoundingBox,
        System_Camera,
        System_Model,
        System_Transform
    >(world);
}

void EngineSystems::update(World& world, Player& player, float deltaTime)
{
    for (auto& system : systems)
        system->update(world, player, deltaTime);
}

void EngineSystems::reset()
{
    for (auto& system : systems)
        system->clear();
}

void EngineSystems::addSystem(World& world, std::unique_ptr<System> system)
{
    System* systemPtr = systems.emplace_back(std::move(system)).get();
    world.observeOnComponentAdded([&world, systemPtr](Entity entity, ComponentTypeId componentId)
    {
        systemPtr->onComponentAdded(world, entity, componentId);
    });
}

template <typename T>
void EngineSystems::addSystem(World& world)
{
    addSystem(world, std::make_unique<T>());
}

template <typename T, typename... T_Systems>
void EngineSystems::addSystems(World& world)
{
    addSystem<T>(world);

    if constexpr (sizeof...(T_Systems) > 0)
    {
        addSystems<T_Systems...>(world);
    }
}