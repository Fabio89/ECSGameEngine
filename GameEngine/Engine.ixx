module;
export module Engine.Core;

import std;
import Engine.Config;
import Engine.Job;

class World;
export using Entity = size_t;
export using ComponentTypeId = std::type_index;
export template <typename T>
struct Component
{
    static const ComponentTypeId typeId;
};

template <typename T>
const ComponentTypeId Component<T>::typeId(typeid(T));

// Archetype
export class Archetype
{
public:
    bool isEmpty() const { return m_componentArrays.empty(); }

    template <typename T>
    void addComponent(Entity entity, T component);

    template <typename T>
    const T& readComponent(Entity entity) const;

    void removeEntity(Entity entity);

private:
    class ComponentArrayBase
    {
    public:
        virtual ~ComponentArrayBase() = default;
        virtual void remove(Entity entity) = 0;
        virtual bool isEmpty() const = 0;
    };

    template <typename T>
    class ComponentArray : public ComponentArrayBase
    {
    public:
        bool isEmpty() const override { return m_components.empty(); }

        void insert(Entity entity, T component);

        void remove(Entity entity) override;

        const T& get(Entity entity) const;
        T& get(Entity entity);

    private:
        std::vector<T> m_components;
        std::unordered_map<Entity, size_t> m_entityToIndex;
        std::unordered_map<size_t, Entity> m_indexToEntity;
    };

    std::unordered_map<ComponentTypeId, std::unique_ptr<ComponentArrayBase>> m_componentArrays;
};

export using ArchetypeChangedCallback = std::function<void(Entity, ComponentTypeId)>;
export using ArchetypeChangedObserverHandle = int;
export ArchetypeChangedObserverHandle generateArchetypeObserverHandle()
{
    static ArchetypeChangedObserverHandle lastValue{-1};
    return ++lastValue;
}

// Definitions

template <typename T>
void Archetype::ComponentArray<T>::insert(Entity entity, T component)
{
    if (m_entityToIndex.contains(entity))
    {
        const size_t index = m_entityToIndex[entity];
        m_components[index] = component;
    }
    else
    {
        const size_t index = m_components.size();
        m_components.push_back(component);
        m_entityToIndex[entity] = index;
        m_indexToEntity[index] = entity;
    }
}

template <typename T>
void Archetype::ComponentArray<T>::remove(Entity entity)
{
    const size_t index = m_entityToIndex[entity];
    const size_t lastIndex = m_components.size() - 1;
    m_components[index] = m_components[lastIndex];
    const Entity lastEntity = m_indexToEntity[lastIndex];
    m_entityToIndex[lastEntity] = index;
    m_indexToEntity[index] = lastEntity;
    m_components.pop_back();
    m_entityToIndex.erase(entity);
    m_indexToEntity.erase(lastIndex);
}

template <typename T>
const T& Archetype::ComponentArray<T>::get(Entity entity) const
{
    if (auto indexIt = m_entityToIndex.find(entity); indexIt != m_entityToIndex.end())
    {
        return m_components.at(indexIt->second);
    }
    static const T invalid{};
    return invalid;
}

template <typename T>
T& Archetype::ComponentArray<T>::get(Entity entity)
{
    return const_cast<T&>(std::as_const(*this).get(entity));
}

template <typename T>
void Archetype::addComponent(Entity entity, T component)
{
    auto& arr = m_componentArrays[Component<T>::typeId];
    if (!arr)
        arr = std::make_unique<ComponentArray<T>>();

    static_cast<ComponentArray<T>*>(arr.get())->insert(entity, component);
}

template <typename T>
const T& Archetype::readComponent(Entity entity) const
{
    static const auto type = Component<T>::typeId;
    if (auto it = m_componentArrays.find(type); it != m_componentArrays.end())
    {
        return static_cast<const ComponentArray<T>*>(it->second.get())->get(entity);
    }
    static const T invalid{};
    return invalid;
}