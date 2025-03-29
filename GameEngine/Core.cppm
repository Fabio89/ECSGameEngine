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

export using TypeId = size_t;
export constexpr TypeId invalidId() { return std::numeric_limits<TypeId>::max(); }

export namespace UniqueIdGenerator
{
    __declspec(dllexport)
    TypeId generateUniqueId()
    {
        static TypeId id = 0;
        log(std::format("Generated unique id: {}", id));
        return id++;
    }

    template <typename T>
    struct __declspec(dllexport) TypeIdGenerator
    {
        inline static const TypeId value = generateUniqueId();
    };
}

// General hash function
consteval size_t hash_fnv1a(std::string_view str)
{
    size_t hash = 2166136261u;
    for (char c : str)
    {
        hash ^= static_cast<size_t>(c);
        hash *= 16777619u;
    }
    return hash;
}

export template <typename T>
consteval size_t getTypeHash()
{
    return hash_fnv1a(__FUNCSIG__); // Use __PRETTY_FUNCTION__ (or __FUNCSIG__ on MSVC)
}

export using Entity = TypeId;

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
struct TypeTraits
{
    static constexpr const char* name = "Unknown";
};

export template <ValidComponentData T>
constexpr const char* getComponentName() { return TypeTraits<T>::name; }

export template <ValidComponentData T>
constexpr ComponentTypeId getComponentType() { return Component<T>::typeId(); }

template <ValidComponentData T>
consteval ComponentTypeId Component<T>::typeId()
{
    return getTypeHash<T>();
}
