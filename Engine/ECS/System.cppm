export module System;
export import Core;
export import Player;
export import World;

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
    void notifyComponentAdded(World&, Entity, TypeId);
    void notifyEntityDestroyed(World&, Entity);

private:
    virtual void onComponentAdded(World& world, Entity entity, TypeId componentTypeId) {}
    virtual void onEntityDestroyed(World& world, Entity entity) {}
    virtual void onUpdate(World& world, Player& player, float deltaTime) {}
};

void System::update(World& world, Player& player, float deltaTime)
{
    onUpdate(world, player, deltaTime);
}

void System::notifyComponentAdded(World& world, Entity entity, TypeId componentTypeId)
{
    onComponentAdded(world, entity, componentTypeId);
}

void System::notifyEntityDestroyed(World& world, Entity entity)
{
    onEntityDestroyed(world, entity);
}
