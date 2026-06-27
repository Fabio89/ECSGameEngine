module;

#ifdef _MSC_VER
#define FUNCTION_SIGNATURE __FUNCSIG__
#else
#define FUNCTION_SIGNATURE __PRETTY_FUNCTION__
#endif

export module Core;
export import Log;
export import std;

export using Int8 = std::int8_t;
export using Int16 = std::int16_t;
export using Int32 = std::int32_t;
export using Int64 = std::int64_t;
export using UInt8 = std::uint8_t;
export using UInt16 = std::uint16_t;
export using UInt32 = std::uint32_t;
export using UInt64 = std::uint64_t;

export template<typename Tag, typename T = UInt32>
struct Id
{
    using ValueType = T;
    static constexpr T invalidValue = std::numeric_limits<T>::max();

    T value{invalidValue};

    [[nodiscard]]
    constexpr bool isValid() const
    {
        return value != invalidValue;
    }

    explicit constexpr operator bool() const
    {
        return isValid();
    }

    auto operator<=>(const Id&) const = default;
};

template<typename Tag, typename T>
struct std::hash<Id<Tag, T>>
{
    constexpr std::size_t operator()(const Id<Tag, T>& id) const noexcept
    {
        return std::hash<T>{}(id.value);
    }
};

template<typename Tag, typename T>
struct std::formatter<Id<Tag, T>>
{
    constexpr auto parse(std::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    auto format(const Id<Tag, T>& id, auto& ctx) const
    {
        return std::format_to(ctx.out(), "{}", id.value);
    }
};

using TypeHash = std::uint64_t;

consteval TypeHash hash_fnv1a(std::string_view str)
{
    TypeHash hash = 14695981039346656037ull;

    for (char c : str)
    {
        hash ^= static_cast<std::uint8_t>(c);
        hash *= 1099511628211ull;
    }

    return hash;
}

template <typename T>
consteval std::size_t getTypeHash()
{
    return hash_fnv1a(FUNCTION_SIGNATURE);
}

export using Entity = Id<struct EntityTag>;

export using TypeId = Id<struct TypeIdTag, std::size_t>;

//------------------------------------------------------------------------------------------------------------------------
// Generic type reflection
//------------------------------------------------------------------------------------------------------------------------

export template<typename T>
constexpr std::string_view getTypeName()
{
    return "<unknown>";
}

export template<typename T>
consteval TypeId getTypeId()
{
    return {getTypeHash<T>()};
}

//------------------------------------------------------------------------------------------------------------------------
// Component
//------------------------------------------------------------------------------------------------------------------------

export template <typename T>
concept ValidComponentData = true;

export struct ComponentBase
{
};

export template <ValidComponentData T>
struct Component : ComponentBase
{
    static consteval TypeId typeId();
    T data;
};

export template <typename T>
concept ValidComponent = std::is_base_of_v<Component<T>, T>;

export template <ValidComponentData T>
constexpr std::string_view getComponentName() { return getTypeName<T>(); }

export template <ValidComponentData T>
constexpr TypeId getComponentType() { return Component<T>::typeId(); }

template <ValidComponentData T>
consteval TypeId Component<T>::typeId()
{
    return {getTypeHash<T>()};
}