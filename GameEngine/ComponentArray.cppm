export module ComponentArray;
import Core;

export class ComponentArrayBase;

export template <ValidComponent T>
class ComponentArray;

class ComponentArrayBase
{
public:
    ComponentArrayBase() = default;
    ComponentArrayBase(const ComponentArrayBase&) = delete;
    ComponentArrayBase(ComponentArrayBase&&) = delete;
    ComponentArrayBase& operator=(const ComponentArrayBase&) = delete;
    ComponentArrayBase& operator=(ComponentArrayBase&&) = delete;
    virtual ~ComponentArrayBase() = default;

    virtual void copy(ComponentArrayBase& other, size_t indexFrom, size_t indexTo) = 0;
    virtual void move(size_t fromIndex, size_t toIndex) = 0;
    virtual std::unique_ptr<ComponentArrayBase> cloneForIndex(size_t index) = 0;
    virtual const ComponentBase& get(size_t index) const = 0;
    virtual ComponentBase& get(size_t index) = 0;
};

template <ValidComponent T>
class ComponentArray final : public ComponentArrayBase
{
public:
    void insert(size_t index, T component);
    
    std::unique_ptr<ComponentArrayBase> cloneForIndex(Entity index) override;

    void copy(ComponentArrayBase& other, size_t indexFrom, size_t indexTo) override;
    void move(size_t fromIndex, size_t toIndex) override;

    const T& get(size_t index) const override;
    T& get(size_t index) override;

private:
    std::vector<T> m_components = std::vector<T>(100, {});
};

template <ValidComponent T>
void ComponentArray<T>::insert(size_t index, T component)
{
    m_components.at(index) = component;
}

template <ValidComponent T>
void ComponentArray<T>::move(size_t fromIndex, size_t toIndex)
{
    m_components[toIndex] = std::move(m_components[fromIndex]);
    m_components[fromIndex] = {};
}

template <ValidComponent T>
[[nodiscard]] std::unique_ptr<ComponentArrayBase> ComponentArray<T>::cloneForIndex(size_t index)
{
    auto componentArray = std::make_unique<ComponentArray>();
    componentArray->m_components[0] = m_components.at(index);
    return componentArray;
}

template <ValidComponent T>
void ComponentArray<T>::copy(ComponentArrayBase& other, size_t indexFrom, size_t indexTo)
{
    ComponentArray& otherComponentArray = static_cast<ComponentArray&>(other);
    m_components.at(indexTo) = otherComponentArray.m_components.at(indexFrom);
}

template <ValidComponent T>
[[nodiscard]] const T& ComponentArray<T>::get(size_t index) const
{
    return m_components.at(index);
}

template <ValidComponent T>
[[nodiscard]] T& ComponentArray<T>::get(size_t index)
{
    return const_cast<T&>(std::as_const(*this).get(index));
}

template <ValidComponent ... Components>
constexpr bool containsType(ComponentTypeId id)
{
    return (... || (Components::typeId() == id));
}
