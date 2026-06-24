export module Properties;
import Core;

// export class PropertyDescriptor
// {
// public:
//     virtual ~PropertyDescriptor() = default;
//
//     virtual std::string_view getName() const = 0;
//
//     virtual void draw(void* component) const = 0;
// };
//
// export template<ValidComponentData T, typename Member>
// class PropertyDescriptorImpl final : public PropertyDescriptor
// {
// public:
//     constexpr PropertyDescriptorImpl(std::string_view name, Member T::* member)
//         : m_name(name), m_member(member) {}
//
//     std::string_view getName() const override
//     {
//         return m_name;
//     }
//
//     void draw(void* component) const override
//     {
//         T& c = *static_cast<Component*>(component);
//         drawProperty(m_name, c.*m_member);
//     }
//
// private:
//     std::string_view m_name;
//     Member T::* m_member;
// };

// export template<ValidComponentData T, typename Member>
// constexpr auto MakeProperty(std::string_view name, Member T::* member)
// {
//     return PropertyDescriptorImpl<T, Member>{name, member};
// }

export
struct PropertyDescriptorBase
{
    virtual ~PropertyDescriptorBase() = default;

    [[nodiscard]]
    virtual TypeId getTypeId() const = 0;

    [[nodiscard]]
    virtual void* get(void* object) const = 0;

    virtual void draw(void* object) const = 0;
};

using DrawerFunc = std::function<void(std::string_view, void*)>;

std::unordered_map<TypeId, DrawerFunc> drawers;

export template<typename T>
void registerDrawer(DrawerFunc drawer)
{
    drawers[getTypeId<T>()] = drawer;
}

export template<typename T, typename M>
struct PropertyDescriptor : PropertyDescriptorBase
{
    std::string_view name{};
    M T::* member;

    constexpr PropertyDescriptor(std::string_view name, M T::* member) : name{name}, member{member} {}

    [[nodiscard]] TypeId getTypeId() const override
    {
        return ::getTypeId<M>();
    }

    void* get(void* object) const override
    {
        return &(static_cast<T*>(object)->*member);
    }

    void draw(void* object) const override
    {
        if (auto it = drawers.find(::getTypeId<M>()); it != drawers.end())
        {
            it->second(name, get(object));
        }
    }
};

export template<typename T>
struct TypeProperties
{
    static constexpr std::tuple list{};
};

template<typename T, typename M>
struct PropertyAccessor
{
    M T::* member;

    constexpr void* get(void* object) const
    {
        return &(static_cast<T*>(object)->*member);
    }
};

export template<typename T, typename M>
constexpr auto makeProperty(std::string_view name, M T::* member)
{
    return PropertyDescriptor<T, M>{name, member};
}

export
constexpr std::array<const PropertyDescriptorBase, 0> makeEmptyPropertyList()
{
    return {};
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