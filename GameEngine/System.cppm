export module Engine:System;
import :Core;
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
