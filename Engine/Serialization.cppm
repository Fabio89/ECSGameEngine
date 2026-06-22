module;
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/prettywriter.h"

export module Serialization;
import Core;
import Math;

export using JsonDocument = rapidjson::Document;
export using JsonObject = rapidjson::Value;

export namespace Json
{
    using rapidjson::MemoryPoolAllocator;
    using rapidjson::SizeType;
    using rapidjson::GenericStringRef;
    
    using rapidjson::kNullType;
    using rapidjson::kFalseType;
    using rapidjson::kTrueType;
    using rapidjson::kObjectType;
    using rapidjson::kArrayType;
    using rapidjson::kStringType;
    using rapidjson::kNumberType;

    using rapidjson::StringBuffer;
    using rapidjson::Writer;
    using rapidjson::PrettyWriter;

    using rapidjson::IStreamWrapper;
    using rapidjson::OStreamWrapper;
    
    template<typename T>
    std::optional<T> toNumber(const JsonObject& j, const char* key);

    JsonObject fromVec2(Vec2 v, MemoryPoolAllocator<>& allocator);
    std::optional<Vec2> toVec2(const JsonObject& j, const char* key);
    JsonObject fromVec3(Vec3 v, MemoryPoolAllocator<>& allocator);
    std::optional<Vec3> toVec3(const JsonObject& j, const char* key);
    JsonObject fromQuat(Quat q, MemoryPoolAllocator<>& allocator);
    std::optional<Quat> toQuat(const JsonObject& j, const char* key);
    std::optional<std::string> toString(const JsonObject& j, const char* key);
    JsonDocument fromString(std::string_view jsonStr);
    JsonDocument fromFile(std::string_view path);
    void toFile(const JsonDocument& document, std::string_view path);
    constexpr int defaultFloatPrecision = 5;
}

export template <ValidComponentData T>
JsonObject serialize(const T&, Json::MemoryPoolAllocator<>&) { return {}; }

export template <ValidComponentData T> 
T deserialize(const JsonObject&) { return T{}; }

template<typename T>
T toNumber(const JsonObject& j);

template<>
int toNumber<int>(const JsonObject& j) { return j.GetInt(); }

template<>
UInt64 toNumber<UInt64>(const JsonObject& j) { return j.GetUint64(); }

template<typename T>
std::optional<T> Json::toNumber(const JsonObject& j, const char* key)
{
    const auto it = j.FindMember(key);
    if (it == j.MemberEnd())
        return std::nullopt;
        
    return ::toNumber<T>(it->value);
}