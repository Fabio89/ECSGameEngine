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

template<typename T>
concept NumericType = std::integral<T> || std::floating_point<T>;

export template<NumericType TargetType, NumericType CurrentType>
TargetType narrow_cast(CurrentType number)
{
    check(std::in_range<TargetType>(number), "Numeric cast overflow!");
    return static_cast<TargetType>(number);
}

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

export template<typename Tag, typename T>
struct std::hash<Id<Tag, T>>
{
    constexpr std::size_t operator()(const Id<Tag, T>& id) const noexcept
    {
        return std::hash<T>{}(id.value);
    }
};

export template<typename Tag, typename T>
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

export struct EntityTag {};
export using Entity = Id<EntityTag>;

export struct TypeIdTag {};
export using TypeId = Id<TypeIdTag, std::size_t>;

//------------------------------------------------------------------------------------------------------------------------
// Generic type reflection
//------------------------------------------------------------------------------------------------------------------------

class TypeRegistry
{
public:
    template<typename T>
    static TypeId id()
    {
        static const TypeId value{next()};
        return value;
    }

private:
    static TypeId next()
    {
        static std::atomic<UInt64> counter = 0;
        return {counter++};
    }
};

export template<typename T>
constexpr std::string_view getTypeName()
{
    return "<unknown>";
}

export template<typename T>
TypeId getTypeId()
{
    using U = std::remove_cvref_t<T>;
    return { TypeRegistry::id<U>() };
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
    T data;
};