export module ComponentArray;
import Core;

export class ComponentArrayBase;

export template <ValidComponentData T>
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

template <ValidComponentData T>
class ComponentArray final : public ComponentArrayBase
{
public:
    T& insert(size_t index, T&& component);
    
    std::unique_ptr<ComponentArrayBase> cloneForIndex(Entity index) override;

    void copy(ComponentArrayBase& other, size_t indexFrom, size_t indexTo) override;
    void move(size_t fromIndex, size_t toIndex) override;

    const Component<T>& get(size_t index) const override;
    Component<T>& get(size_t index) override;

private:
    std::vector<Component<T>> m_components = std::vector<Component<T>>(100, {});
};

template <ValidComponentData T>
T& ComponentArray<T>::insert(size_t index, T&& component)
{
    return m_components.at(index).data = std::forward<T>(component);
}

template <ValidComponentData T>
void ComponentArray<T>::move(size_t fromIndex, size_t toIndex)
{
    m_components[toIndex] = std::move(m_components[fromIndex]);
    m_components[fromIndex] = {};
}

template <ValidComponentData T>
[[nodiscard]] std::unique_ptr<ComponentArrayBase> ComponentArray<T>::cloneForIndex(size_t index)
{
    auto componentArray = std::make_unique<ComponentArray>();
    componentArray->m_components[0] = m_components.at(index);
    return componentArray;
}

template <ValidComponentData T>
void ComponentArray<T>::copy(ComponentArrayBase& other, size_t indexFrom, size_t indexTo)
{
    ComponentArray& otherComponentArray = static_cast<ComponentArray&>(other);
    m_components.at(indexTo) = otherComponentArray.m_components.at(indexFrom);
}

template <ValidComponentData T>
[[nodiscard]] const Component<T>& ComponentArray<T>::get(size_t index) const
{
    return m_components.at(index);
}

template <ValidComponentData T>
[[nodiscard]] Component<T>& ComponentArray<T>::get(size_t index)
{
    return const_cast<Component<T>&>(std::as_const(*this).get(index));
}

template <ValidComponentData ... Components>
constexpr bool containsType(ComponentTypeId id)
{
    return (... || (Component<Components>::typeId() == id));
}
