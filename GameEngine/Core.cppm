export module Core;
import Log;
import std;

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

