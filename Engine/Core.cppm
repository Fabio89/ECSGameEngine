module;

#include "EngineExport.h"

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

export using TypeId = std::size_t;
export constexpr TypeId invalidId() { return std::numeric_limits<TypeId>::max(); }

export namespace UniqueIdGenerator
{
    ENGINE_API
    TypeId generateUniqueId()
    {
        static TypeId id = 0;
        log(std::format("Generated unique id: {}", id));
        return id++;
    }

    template <typename T>
    struct ENGINE_API TypeIdGenerator
    {
        inline static const TypeId value = generateUniqueId();
    };
}

// General hash function
consteval std::size_t hash_fnv1a(std::string_view str)
{
    std::size_t hash = 2166136261u;
    for (char c : str)
    {
        hash ^= static_cast<std::size_t>(c);
        hash *= 16777619u;
    }
    return hash;
}

export template <typename T>
consteval std::size_t getTypeHash()
{
    return hash_fnv1a(FUNCTION_SIGNATURE);
}

export using Entity = TypeId;

//------------------------------------------------------------------------------------------------------------------------
// Generic type reflection
//------------------------------------------------------------------------------------------------------------------------

export struct TypeInfo
{
    TypeId id{invalidId()};
    std::string_view name{};
};

export template<typename T>
struct TypeTraits
{
    static constexpr std::string_view name = "Unknown";
};

export template<typename T>
consteval TypeId getTypeId()
{
    return getTypeHash<T>();
}

export template<typename T>
constexpr TypeInfo getTypeInfo()
{
    return
    {
        .id = getTypeId<T>(),
        .name = TypeTraits<T>::name
    };
}

//------------------------------------------------------------------------------------------------------------------------
// Component
//------------------------------------------------------------------------------------------------------------------------

export using ComponentTypeId = TypeId;

export template <typename T>
concept ValidComponentData = true;

export struct ComponentBase
{
};

export template <ValidComponentData T>
struct Component : ComponentBase
{
    static consteval ComponentTypeId typeId();
    T data;
};

export template <typename T>
concept ValidComponent = std::is_base_of_v<Component<T>, T>;

export template <ValidComponentData T>
constexpr std::string_view getComponentName() { return TypeTraits<T>::name; }

export template <ValidComponentData T>
constexpr ComponentTypeId getComponentType() { return Component<T>::typeId(); }

template <ValidComponentData T>
consteval ComponentTypeId Component<T>::typeId()
{
    return getTypeHash<T>();
}