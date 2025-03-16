export module Engine:Core;
import std;

export template<typename T>
void logError(T&& message)
{
    std::cerr << "\033[31m" << "ERROR: " << std::forward<T>(message) << "\033[0m\n";
}

export enum class ErrorType
{
    Warning,
    Error,
    FatalError
};

export template<typename T>
bool check(bool expression, T&& message, ErrorType type = ErrorType::Error)
{
    if (!expression)
    {
        logError(std::forward<T>(message));
        if (type == ErrorType::FatalError)
            std::abort();
        return false;
    }
    return true;
}

export template<typename T>
void fatalError(T&& message)
{
    logError(std::forward<T>(message));
    std::abort();
}

export using Entity = size_t;
export using ComponentTypeId = std::type_index;

export template<typename T>
concept ValidComponentData = true;

export template <ValidComponentData T>
struct Component
{
    static const ComponentTypeId typeId;
};

export template<typename T>
concept ValidComponent = std::is_base_of_v<Component<T>, T>;

template <ValidComponentData T>
const ComponentTypeId Component<T>::typeId(typeid(T));

export template<ValidComponent T>
std::string getComponentName() { return T::typeId.name(); }

// Archetype
export class Archetype
{
    class ComponentArrayBase;
    
public:
    using ComponentArrayMap = std::map<ComponentTypeId, std::unique_ptr<ComponentArrayBase>>;
    using ComponentRange = std::ranges::elements_view<std::ranges::ref_view<const ComponentArrayMap>, 0>;
    
    [[nodiscard]] bool isEmpty() const { return m_componentArrays.empty(); }

    template <ValidComponent T>
    void addComponent(Entity entity, T component);

    template <ValidComponent T>
    [[nodiscard]] const T& readComponent(Entity entity) const;
    
    void removeEntity(Entity entity);

    Archetype cloneForEntity(Entity entity)
    {
        Archetype newArchetype;
        for (const auto& [componentType, componentArray] : m_componentArrays)
        {
            newArchetype.m_componentArrays[componentType] = componentArray->cloneForEntity(entity);
        }
        return newArchetype;
    }

    void steal(Archetype& other, Entity entity)
    {
        for (auto& [componentType, componentArray] : other.m_componentArrays)
        {
            m_componentArrays[componentType]->steal(*componentArray, entity);
        }
    }

    [[nodiscard]] ComponentRange getComponentTypes() const { return m_componentArrays | std::views::keys; }

private:
    class ComponentArrayBase
    {
    public:
        ComponentArrayBase() = default;
        ComponentArrayBase(const ComponentArrayBase&) = delete;
        ComponentArrayBase(ComponentArrayBase&&) = delete;
        ComponentArrayBase& operator=(const ComponentArrayBase&) = delete;
        ComponentArrayBase& operator=(ComponentArrayBase&&) = delete;
        virtual ~ComponentArrayBase() = default;
        
        virtual void steal(ComponentArrayBase& other, Entity entity) = 0;
        virtual void remove(Entity entity) = 0;
        [[nodiscard]] virtual bool isEmpty() const = 0;
        virtual std::unique_ptr<ComponentArrayBase> cloneForEntity(Entity entity) = 0;
    };

    template <ValidComponent T>
    class ComponentArray final : public ComponentArrayBase
    {
    public:
        [[nodiscard]] bool isEmpty() const override { return m_components.empty(); }

        void insert(Entity entity, T component);

        void remove(Entity entity) override;

        std::unique_ptr<ComponentArrayBase> cloneForEntity(Entity entity) override
        {
            const size_t index = m_entityToIndex.find(entity)->second;
            auto componentArray = std::make_unique<ComponentArray>();
            componentArray->m_components = {m_components.at(index)};
            componentArray->m_entityToIndex[entity] = 0;
            componentArray->m_indexToEntity[0] = entity;
            return componentArray;
        }

        void steal(ComponentArrayBase& other, Entity entity) override
        {
            ComponentArray& otherComponentArray = static_cast<ComponentArray&>(other);

            m_components.push_back(otherComponentArray.m_components.at(otherComponentArray.m_entityToIndex[entity]));
            m_entityToIndex[entity] = m_components.size() - 1;
            m_indexToEntity[m_components.size() - 1] = entity;
            other.remove(entity);
        }

        [[nodiscard]] const T& get(Entity entity) const;
        T& get(Entity entity);

    private:
        std::vector<T> m_components;
        std::map<Entity, size_t> m_entityToIndex;
        std::map<size_t, Entity> m_indexToEntity;
    };

    std::map<ComponentTypeId, std::unique_ptr<ComponentArrayBase>> m_componentArrays;
};

export using ArchetypeChangedCallback = std::function<void(Entity, ComponentTypeId)>;
export using ArchetypeChangedObserverHandle = int;

export ArchetypeChangedObserverHandle generateArchetypeObserverHandle()
{
    static ArchetypeChangedObserverHandle lastValue{-1};
    return ++lastValue;
}

// Definitions

template <ValidComponent T>
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

template <ValidComponent T>
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

template <ValidComponent T>
const T& Archetype::ComponentArray<T>::get(Entity entity) const
{
    if (auto indexIt = m_entityToIndex.find(entity); indexIt != m_entityToIndex.end())
    {
        return m_components.at(indexIt->second);
    }
    static const T invalid{};
    return invalid;
}

template <ValidComponent T>
T& Archetype::ComponentArray<T>::get(Entity entity)
{
    return const_cast<T&>(std::as_const(*this).get(entity));
}

template <ValidComponent T>
void Archetype::addComponent(Entity entity, T component)
{
    auto& arr = m_componentArrays[Component<T>::typeId];
    if (!arr)
        arr = std::make_unique<ComponentArray<T>>();

    static_cast<ComponentArray<T>*>(arr.get())->insert(entity, component);
}

template <ValidComponent T>
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

// const ComponentBase& Archetype::readComponent(Entity entity, ComponentTypeId componentType) const
// {
//     if (auto it = m_componentArrays.find(componentType); it != m_componentArrays.end())
//     {
//         ComponentArrayBase* componentArray = it->second.get();
//         return componentArray->get(entity);
//     }
//
//     struct InvalidComponent : Component<InvalidComponent> {};
//     static const InvalidComponent invalid{};
//     return invalid;  
// }