export module Serialization;
export import Wrapper.RapidJson;
import Math;
import std;

export namespace Json
{
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

export template <typename T>
JsonObject serialize(const T&, Json::MemoryPoolAllocator<>&) { return {}; }

export template <typename T> 
T deserialize(const JsonObject&) { return T{}; }