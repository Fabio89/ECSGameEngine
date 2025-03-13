export module Engine:System;
import :Decl;
import std;

export class World;

export class System
{
public:
    virtual ~System() = default;
    virtual void update(float deltaTime);
    void addUpdateFunction(std::function<void(float)> func);

    virtual void onComponentAdded(World&, Entity, ComponentTypeId)
    {
    }

private:
    std::vector<std::function<void(float)>> m_updateFunctions;
};
