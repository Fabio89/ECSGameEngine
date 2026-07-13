export module Properties;
import Core;
import World;

export using PropertyValue = std::any;

export struct PropertyDescriptorBase
{
    virtual ~PropertyDescriptorBase() = default;

    [[nodiscard]]
    virtual TypeId getTypeId() const = 0;

    [[nodiscard]]
    virtual std::string_view getName() const = 0;

    [[nodiscard]]
    virtual void* get(void* object) const = 0;

    virtual void set(void* object, const PropertyValue& value) const = 0;

    virtual void set(BaseEdit& object, const PropertyValue& value) const = 0;

    virtual PropertyValue copy(const void* object) const = 0;
};

export template<typename T, typename M>
struct PropertyDescriptor : PropertyDescriptorBase
{
    std::string_view name{};
    M T::* member;

    constexpr PropertyDescriptor(std::string_view name, M T::* member) : name{name}, member{member} {}

    [[nodiscard]] TypeId getTypeId() const override { return ::getTypeId<M>(); }

    [[nodiscard]] std::string_view getName() const override { return name; }

    void* get(void* object) const override
    {
        return &(static_cast<T*>(object)->*member);
    }

    void set(void* object, const PropertyValue& value) const override
    {
        auto& instance = *static_cast<T*>(object);
        instance.*member = std::any_cast<M>(value);
    }

    void set(BaseEdit& object, const PropertyValue& value) const override
    {
        Edit<T> instance = object.as<T>();
        instance.operator->()->*member = std::any_cast<M>(value);
    }

    PropertyValue copy(const void* object) const override
    {
        return static_cast<const T*>(object)->*member;
    }
};

export template<typename T>
struct TypeProperties
{
    static constexpr std::tuple list{};
};

export template<typename T, typename M>
constexpr auto makeProperty(std::string_view name, M T::* member)
{
    return PropertyDescriptor<T, M>{name, member};
}

template<typename Tuple, std::size_t... I>
std::generator<const PropertyDescriptorBase&> generatePropertiesImpl(const Tuple& tuple, std::index_sequence<I...>)
{
    (co_yield static_cast<const PropertyDescriptorBase&>(std::get<I>(tuple)), ...);
}

export template<typename Tuple>
std::generator<const PropertyDescriptorBase&> generateProperties(const Tuple& tuple)
{
    return generatePropertiesImpl(tuple, std::make_index_sequence<std::tuple_size_v<Tuple> >{});
}
