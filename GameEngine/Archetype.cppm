export module Archetype;
export import Core;
export import Log;
import ComponentArray;

//------------------------------------------------------------------------------------------------------------------------
// Archetype
//------------------------------------------------------------------------------------------------------------------------
export class Archetype
{
public:
    using ComponentArrayMap = std::unordered_map<ComponentTypeId, std::unique_ptr<ComponentArrayBase>>;
    using ComponentRange = std::ranges::elements_view<std::ranges::ref_view<const ComponentArrayMap>, 0>;

    bool isEmpty() const;

    template <ValidComponent T>
    void addComponent(Entity entity, T component);

    template <ValidComponent T>
    const T& readComponent(Entity entity) const;

    const ComponentBase& readComponent(Entity entity, ComponentTypeId componentType) const;

    void removeEntity(Entity entity);

    Archetype cloneForEntity(Entity entity);

    void steal(Archetype& other, Entity entity);

    ComponentRange getComponentTypes() const { return m_componentArrays | std::views::keys; }

    template <ValidComponent... Components>
    bool matches() const;

    template <ValidComponent ... Components>
    std::generator<std::tuple<Entity, const Components&...>> view() const;

    template <ValidComponent ... Components>
    std::generator<std::tuple<Entity, Components&...>> view();

private:
    template <ValidComponent T>
    const ComponentArray<T>& getComponentArray() { return static_cast<const ComponentArray<T>&>(*m_componentArrays.at(T::typeId())); }

    std::unordered_map<ComponentTypeId, std::unique_ptr<ComponentArrayBase>> m_componentArrays;
    std::unordered_map<Entity, size_t> m_entityToIndex;
    std::vector<Entity> m_indexToEntity;

    template <ValidComponent ... Components>
    class ViewIterator;
};

export using ArchetypeChangedCallback = std::function<void(Entity, ComponentTypeId)>;
export using ArchetypeChangedObserverHandle = int;

export ArchetypeChangedObserverHandle generateArchetypeObserverHandle()
{
    static ArchetypeChangedObserverHandle lastValue{-1};
    return ++lastValue;
}

//------------------------------------------------------------------------------------------------------------------------
// Archetype - Implementation
//------------------------------------------------------------------------------------------------------------------------

template <ValidComponent ... Components>
class Archetype::ViewIterator
{
public:
    ViewIterator(const Archetype* archetype, size_t index);

    // Dereference to get the current entity and its components
    auto operator*() const;

    // Move to the next valid entity
    ViewIterator& operator++();

    bool operator!=(const ViewIterator& other) const;

private:
    const Archetype* m_archetype;
    size_t m_index;
};

template <ValidComponent ... Components>
Archetype::ViewIterator<Components...>::ViewIterator(const Archetype* archetype, size_t index)
    : m_archetype(archetype), m_index(index)
{
}

template <ValidComponent ... Components>
auto Archetype::ViewIterator<Components...>::operator*() const
{
    Entity entity = m_archetype->m_indexToEntity[m_index];
    return std::tuple<Entity, const Components&...>(entity, m_archetype->readComponent<Components>(entity)...);
}

template <ValidComponent ... Components>
Archetype::ViewIterator<Components...>& Archetype::ViewIterator<Components...>::operator++()
{
    do
    {
        ++m_index;
    }
    while (m_index < m_archetype->m_indexToEntity.size() && !m_archetype->matches<Components...>());
    return *this;
}

template <ValidComponent ... Components>
bool Archetype::ViewIterator<Components...>::operator!=(const ViewIterator& other) const
{
    return m_index != other.m_index;
}

template <ValidComponent T>
void Archetype::addComponent(Entity entity, T component)
{
    auto& arr = m_componentArrays[T::typeId()];
    if (!arr)
        arr = std::make_unique<ComponentArray<T>>();

    size_t index;
    if (auto it = m_entityToIndex.find(entity); it != m_entityToIndex.end())
    {
        index = it->second;
    }
    else
    {
        index = m_indexToEntity.size();
        m_entityToIndex.insert_or_assign(entity, index);
        m_indexToEntity.push_back(entity);
    }
    static_cast<ComponentArray<T>&>(*arr).insert(index, component);
}

template <ValidComponent T>
[[nodiscard]] const T& Archetype::readComponent(Entity entity) const
{
    if (auto indexIt = m_entityToIndex.find(entity); indexIt != m_entityToIndex.end())
    {
        if (auto it = m_componentArrays.find(T::typeId()); it != m_componentArrays.end())
        {
            return static_cast<const ComponentArray<T>&>(*it->second).get(indexIt->second);
        }
    }

    static const T invalid{};
    return invalid;
}

template <ValidComponent ... Components>
[[nodiscard]] bool Archetype::matches() const
{
    return (... && (m_componentArrays.contains(Components::typeId())));
}

template <ValidComponent ... Components>
std::generator<std::tuple<Entity, const Components&...>> Archetype::view() const
{
    for (Entity entity : m_indexToEntity)
    {
        co_yield std::tuple<Entity, const Components&...>(entity, readComponent<Components>(entity)...);
    }
}

template <ValidComponent ... Components>
std::generator<std::tuple<Entity, Components&...>> Archetype::view()
{
    for (Entity entity : m_indexToEntity)
    {
        co_yield std::tuple<Entity, Components&...>(entity, const_cast<Components&>(readComponent<Components>(entity))...);
    }
}

[[nodiscard]] bool Archetype::isEmpty() const
{
    return m_componentArrays.empty();
}

[[nodiscard]] const ComponentBase& Archetype::readComponent(Entity entity, ComponentTypeId componentType) const
{
    if (auto indexIt = m_entityToIndex.find(entity); indexIt != m_entityToIndex.end())
    {
        if (auto it = m_componentArrays.find(componentType); it != m_componentArrays.end())
        {
            return it->second->get(indexIt->second);
        }
    }

    static constexpr ComponentBase invalid{};
    return invalid;
}

void Archetype::removeEntity(Entity entity)
{
    if (auto indexIt = m_entityToIndex.find(entity); indexIt != m_entityToIndex.end())
    {
        const size_t indexToRemove = indexIt->second;
        const size_t lastEntityIndex = m_indexToEntity.size() - 1;
        const Entity entityToSwapFor = m_indexToEntity.back();
        m_entityToIndex.erase(entity);
        m_entityToIndex[entityToSwapFor] = indexToRemove;
        m_indexToEntity[indexToRemove] = entityToSwapFor;

        for (auto& componentArray : m_componentArrays | std::views::values)
        {
            // Swap the entity we want to remove with the last item in the array
            componentArray->move(lastEntityIndex, indexToRemove);
        }
    }
}

[[nodiscard]] Archetype Archetype::cloneForEntity(Entity entity)
{
    Archetype newArchetype;
    if (auto indexIt = m_entityToIndex.find(entity); indexIt != m_entityToIndex.end())
    {
        newArchetype.m_entityToIndex[entity] = indexIt->second;
        newArchetype.m_indexToEntity.push_back(entity);

        for (const auto& [componentType, componentArray] : m_componentArrays)
        {
            newArchetype.m_componentArrays[componentType] = componentArray->cloneForIndex(indexIt->second);
        }
    }

    return newArchetype;
}

void Archetype::steal(Archetype& other, Entity entity)
{
    if (m_entityToIndex.contains(entity))
    {
        report(std::format("Entity {} already exists in archetype", entity));
        return;
    }

    if (auto fromIndexIt = other.m_entityToIndex.find(entity); fromIndexIt != other.m_entityToIndex.end())
    {
        const size_t indexToRemoveFromOther = fromIndexIt->second;
        const size_t toIndex = m_entityToIndex[entity] = m_indexToEntity.size();
        m_indexToEntity.push_back(entity);

        const Entity entityToSwapFor = other.m_indexToEntity.back();
        const size_t otherArchetypeLastEntityIndex = other.m_indexToEntity.size() - 1;
        other.m_entityToIndex.erase(fromIndexIt);
        other.m_entityToIndex[entityToSwapFor] = indexToRemoveFromOther;
        other.m_indexToEntity[indexToRemoveFromOther] = entityToSwapFor;
        other.m_indexToEntity.pop_back();

        for (auto& [componentType, componentArray] : other.m_componentArrays)
        {
            m_componentArrays[componentType]->copy(*componentArray, indexToRemoveFromOther, toIndex);
            componentArray->move(otherArchetypeLastEntityIndex, indexToRemoveFromOther);
        }
    }
}
