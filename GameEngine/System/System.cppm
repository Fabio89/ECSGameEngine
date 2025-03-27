export module System;
import Ecs;
export import Player;
export import World;
import std;

export class System
{
public:
    virtual ~System() = default;
    System() = default;
    System(const System&) = delete;
    System& operator=(const System&) = delete;
    System(System&&) noexcept = delete;
    System& operator=(System&&) noexcept = delete;
    
    void update(World& world, Player& player, float deltaTime);
    void notifyComponentAdded(World&, Entity, ComponentTypeId);

private:
    virtual void onComponentAdded(World& world, Entity entity, ComponentTypeId componentTypeId);
    virtual void onUpdate(World& world, Player& player, float deltaTime);
};

void System::update(World& world, Player& player, float deltaTime)
{
    onUpdate(world, player, deltaTime);
}

void System::notifyComponentAdded(World& world, Entity entity, ComponentTypeId componentTypeId)
{
    onComponentAdded(world, entity, componentTypeId);
}

void System::onComponentAdded([[maybe_unused]] World& world, [[maybe_unused]] Entity entity, [[maybe_unused]] ComponentTypeId componentTypeId)
{
}

void System::onUpdate([[maybe_unused]] World& world, [[maybe_unused]] Player& player, [[maybe_unused]] float deltaTime)
{
}
