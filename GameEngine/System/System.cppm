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
    void addUpdateFunction(std::function<void(float)> func);
    void clear();
    
    virtual void onComponentAdded(World&, Entity, ComponentTypeId)
    {
    }

private:
    virtual void onUpdate(World& world, Player& player, float deltaTime) {}
    std::vector<std::function<void(float)>> m_updateFunctions;
};

void System::clear()
{
    m_updateFunctions.clear();
}

void System::update(World& world, Player& player, float deltaTime)
{
    for (auto& func : m_updateFunctions)
    {
        func(deltaTime);
    }
    onUpdate(world, player, deltaTime);
}

void System::addUpdateFunction(std::function<void(float)> func)
{
    m_updateFunctions.push_back(std::move(func));
}
