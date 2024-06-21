module Engine.Core;
import Engine.Config;

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

void Archetype::removeEntity(Entity entity)
{
    std::vector<ComponentTypeId> toRemove;
    for (auto& pair : m_componentArrays)
    {
        pair.second->remove(entity);
        if (pair.second->isEmpty())
        {
            toRemove.push_back(pair.first);
        }
    }

    for (ComponentTypeId index : toRemove)
    {
        m_componentArrays.erase(index);
    }
}
