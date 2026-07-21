export module Core:Enum;
import std;

export template<typename E>
struct EnableBitMaskOperators : std::false_type {};

template<typename E>
concept BitMaskEnum = std::is_enum_v<E> && EnableBitMaskOperators<E>::value;

export template<BitMaskEnum E>
constexpr E operator|(E lhs, E rhs)
{
    using U = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<U>(lhs) | static_cast<U>(rhs));
}

export template<BitMaskEnum E>
constexpr E operator&(E lhs, E rhs)
{
    using U = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<U>(lhs) & static_cast<U>(rhs));
}

export template<BitMaskEnum E>
constexpr E& operator|=(E& lhs, E rhs)
{
    return lhs = lhs | rhs;
}

export template<BitMaskEnum E>
constexpr bool hasFlag(E value, E flag)
{
    using U = std::underlying_type_t<E>;
    return (static_cast<U>(value) & static_cast<U>(flag)) == static_cast<U>(flag);
}

export template<BitMaskEnum E>
constexpr bool hasAnyFlag(E value)
{
    using U = std::underlying_type_t<E>;
    return static_cast<U>(value) != 0;
}