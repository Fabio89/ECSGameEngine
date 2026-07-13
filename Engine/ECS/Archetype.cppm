export module Archetype;
export import Core;
export import Log;
import ComponentArray;

template<bool Const, ValidComponentData... Components>
class ArchetypeViewImpl;

export template<ValidComponentData... Components>
using ArchetypeView = ArchetypeViewImpl<false, Components...>;

export template<ValidComponentData... Components>
using ConstArchetypeView = ArchetypeViewImpl<true, Components...>;

//------------------------------------------------------------------------------------------------------------------------
// Archetype
//------------------------------------------------------------------------------------------------------------------------
export class Archetype
{
public:
    Archetype() = default;
    Archetype(const Archetype&) = delete;
    Archetype& operator=(const Archetype&) = delete;
    Archetype(Archetype&&) noexcept = default;
    Archetype& operator=(Archetype&&) noexcept = default;
    
    using ComponentArrayMap = std::unordered_map<TypeId, std::unique_ptr<ComponentArrayBase>>;
    using ComponentRange = std::ranges::elements_view<std::ranges::ref_view<const ComponentArrayMap>, 0>;

    Int32 getSize() const { return narrow_cast<Int32>(m_indexToEntity.size()); }
    [[nodiscard]] bool isEmpty() const;

    template <ValidComponentData T>
    T& addComponent(Entity entity, T&& component);

    template <ValidComponentData T>
    [[nodiscard]] const T& readComponent(Entity entity) const;

    [[nodiscard]] const ComponentBase& readComponent(Entity entity, TypeId componentType) const;

    template<ValidComponentData T>
    T& getComponentAt(Int32 index);

    template<ValidComponentData T>
    const T& getComponentAt(Int32 index) const;

    Entity getEntityAt(Int32 index) const;

    void removeEntity(Entity entity);

    Archetype cloneForEntity(Entity entity);

    void steal(Archetype& other, Entity entity);

    [[nodiscard]] ComponentRange getComponentTypes() const { return m_componentArrays | std::views::keys; }

    template <ValidComponentData... Components>
    [[nodiscard]] bool matches() const;

    template<ValidComponentData... Components>
    auto view() const { return ConstArchetypeView<Components...>{*this}; }

    template<ValidComponentData... Components>
    auto view() { return ArchetypeView<Components...>{*this}; }

private:
    std::unordered_map<TypeId, std::unique_ptr<ComponentArrayBase>> m_componentArrays;
    std::unordered_map<Entity, std::size_t> m_entityToIndex;
    std::vector<Entity> m_indexToEntity;
};

export using ArchetypeChangedCallback = std::function<void(Entity, TypeId)>;

//------------------------------------------------------------------------------------------------------------------------
// Archetype - Implementation
//------------------------------------------------------------------------------------------------------------------------

template <bool Const, ValidComponentData... Components>
class ViewIterator
{
public:
    using ArchetypeType = std::conditional_t<Const, const Archetype, Archetype>;

    ViewIterator(ArchetypeType& archetype, Int32 index)
        : m_archetype(archetype),
          m_index(index) {}

    auto operator*() const
    {
        const Entity entity = m_archetype.getEntityAt(m_index);
        return std::tuple<Entity, std::conditional_t<Const, const Components&, Components&>...>{entity, m_archetype.template getComponentAt<Components>(m_index)...};
    }

    ViewIterator& operator++()
    {
        ++m_index;
        return *this;
    }

    bool operator==(const ViewIterator& other) const
    {
        return m_index == other.m_index;
    }

private:
    ArchetypeType& m_archetype;
    Int32 m_index;
};

[[nodiscard]] bool Archetype::isEmpty() const
{
    return m_entityToIndex.empty();
}

template<bool Const, ValidComponentData... Components>
class ArchetypeViewImpl
{
public:
    using Iterator = ViewIterator<Const, Components...>;
    using ArchetypeType = std::conditional_t<Const, const Archetype, Archetype>;

    explicit ArchetypeViewImpl(ArchetypeType& archetype) : m_archetype{archetype} {}

    auto begin() { return Iterator{m_archetype, 0}; }
    auto end() { return Iterator{m_archetype, m_archetype.getSize()}; }

private:
    ArchetypeType& m_archetype;
};

template<ValidComponentData T>
T& Archetype::addComponent(Entity entity, T&& component)
{
    auto& arr = m_componentArrays[getTypeId<T>()];
    if (!arr)
        arr = std::make_unique<ComponentArray<T>>();

    std::size_t index;
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
    return static_cast<ComponentArray<T>&>(*arr).insert(index, std::forward<T>(component));
}

template <ValidComponentData T>
[[nodiscard]] const T& Archetype::readComponent(Entity entity) const
{
    if (auto indexIt = m_entityToIndex.find(entity); indexIt != m_entityToIndex.end())
    {
        if (auto it = m_componentArrays.find(getTypeId<T>()); it != m_componentArrays.end())
        {
            return static_cast<const ComponentArray<T>&>(*it->second).get(indexIt->second).data;
        }
    }

    static const T invalid{};
    return invalid;
}

[[nodiscard]] const ComponentBase& Archetype::readComponent(Entity entity, TypeId componentType) const
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

template<ValidComponentData T>
T& Archetype::getComponentAt(Int32 index)
{
    return const_cast<T&>(std::as_const(*this).getComponentAt<T>(index));
}

template<ValidComponentData T>
const T& Archetype::getComponentAt(Int32 index) const
{
    auto& array = static_cast<ComponentArray<T>&>(*m_componentArrays.at(getTypeId<T>()));
    return array.get(index).data;
}

Entity Archetype::getEntityAt(Int32 index) const
{
    return m_indexToEntity.at(narrow_cast<std::size_t>(index));
}

void Archetype::removeEntity(Entity entity)
{
    if (auto indexIt = m_entityToIndex.find(entity); indexIt != m_entityToIndex.end())
    {
        const std::size_t indexToRemove = indexIt->second;
        const Entity entityToSwapFor = m_indexToEntity.back();
        m_entityToIndex.erase(entity);

        if (const std::size_t lastEntityIndex = m_indexToEntity.size() - 1; lastEntityIndex > indexToRemove)
        {
            m_entityToIndex[entityToSwapFor] = indexToRemove;
            m_indexToEntity[indexToRemove] = entityToSwapFor;
            for (auto& componentArray : m_componentArrays | std::views::values)
            {
                // Swap the entity we want to remove with the last item in the array
                componentArray->move(lastEntityIndex, indexToRemove);
            }
        }
        m_indexToEntity.pop_back();
    }
}

[[nodiscard]] Archetype Archetype::cloneForEntity(Entity entity)
{
    Archetype newArchetype;
    if (auto indexIt = m_entityToIndex.find(entity); indexIt != m_entityToIndex.end())
    {
        newArchetype.m_entityToIndex[entity] = 0;
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
        const std::size_t indexToRemoveFromOther = fromIndexIt->second;
        const std::size_t toIndex = m_entityToIndex[entity] = m_indexToEntity.size();
        m_indexToEntity.push_back(entity);

        const Entity entityToSwapFor = other.m_indexToEntity.back();
        const std::size_t otherArchetypeLastEntityIndex = other.m_indexToEntity.size() - 1;
        other.m_entityToIndex.erase(fromIndexIt);
        if (entityToSwapFor != entity)
        {
            other.m_entityToIndex[entityToSwapFor] = indexToRemoveFromOther;
            other.m_indexToEntity[indexToRemoveFromOther] = entityToSwapFor;
        }
        other.m_indexToEntity.pop_back();

        for (auto& [componentType, componentArray] : other.m_componentArrays)
        {
            m_componentArrays[componentType]->copy(*componentArray, indexToRemoveFromOther, toIndex);
            componentArray->move(otherArchetypeLastEntityIndex, indexToRemoveFromOther);
        }
    }
}

template <ValidComponentData ... Components>
[[nodiscard]] bool Archetype::matches() const
{
    return (... && (m_componentArrays.contains(getTypeId<Components>())));
}
