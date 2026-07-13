export module Engine.ComponentAccess;
import Core;
import Engine.DirtyTracker;

export template<typename T>
struct Read
{
    const T& component;

    const T* operator->() const
    {
        return &component;
    }
};

export template<typename T>
class Edit
{
public:
    Edit(Entity entity, T& component, DirtyTracker& dirtyTracker) : m_entity{entity}, m_component{component}, m_dirtyTracker{dirtyTracker} {}
    ~Edit();

    T* operator->() { return &m_component; }
    const T* operator->() const { return &m_component; }

    T& operator*() { return m_component; }
    const T& operator*() const { return m_component; }

    T& operator*() && = delete;
    const T& operator*() const && = delete;

    Edit& operator=(const T& data)
    {
        m_component = data;
        return *this;
    }

    Edit& operator=(T&& data)
    {
        m_component = std::move(data);
        return *this;
    }

    template<typename M>
    void setProperty(M T::* member, std::any value)
    {
        m_component.*member = std::any_cast<M>(value);
    }

    const T& get() const { return m_component; }
private:
    Entity m_entity;
    T& m_component;
    DirtyTracker& m_dirtyTracker;
};

export class BaseEdit
{
public:
    BaseEdit(Entity entity, ComponentBase& component, DirtyTracker& dirtyTracker) : m_entity{entity}, m_component{component}, m_dirtyTracker{dirtyTracker} {}

    template<ValidComponentData T>
    Edit<T> as() const { return Edit<T>{m_entity, static_cast<Component<T>&>(m_component).data, m_dirtyTracker}; }

private:
    Entity m_entity;
    ComponentBase& m_component;
    DirtyTracker& m_dirtyTracker;
};

export template<typename T>
struct AccessTraits
{
    using ComponentType = T;
    using AccessType = const T&;
    static constexpr bool writable = false;
};

template<typename T>
struct AccessTraits<Read<T>>
{
    using ComponentType = T;
    using AccessType = const T&;
    static constexpr bool writable = false;
};

template<typename T>
struct AccessTraits<Edit<T>>
{
    using ComponentType = T;
    using AccessType = Edit<T>;
    static constexpr bool writable = true;
};

template<typename T>
Edit<T>::~Edit()
{
    m_dirtyTracker.markDirty<T>(m_entity);
}