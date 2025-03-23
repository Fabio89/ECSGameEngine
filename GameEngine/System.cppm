export module System;
import Ecs;
import std;

export class World;

export class System
{
public:
    virtual ~System() = default;
    virtual void update(float deltaTime);
    void addUpdateFunction(std::function<void(float)> func);
    void clear();
    
    virtual void onComponentAdded(World&, Entity, ComponentTypeId)
    {
    }

private:
    std::vector<std::function<void(float)>> m_updateFunctions;
};

void System::clear()
{
    m_updateFunctions.clear();
}

void System::update(float deltaTime)
{
    for (auto& func : m_updateFunctions)
    {
        func(deltaTime);
    }
}

void System::addUpdateFunction(std::function<void(float)> func)
{
    m_updateFunctions.push_back(std::move(func));
}
