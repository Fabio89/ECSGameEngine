export module Engine:Serialization;
import :Core;
import :Math;
export import Wrapper.RapidJson;

export namespace Json
{
    JsonObject fromVec2(vec3 v, MemoryPoolAllocator<> allocator);
    std::optional<vec2> toVec2(const JsonObject& j, const char* key);
    JsonObject fromVec3(vec3 v, MemoryPoolAllocator<> allocator);
    std::optional<vec3> toVec3(const JsonObject& j, const char* key);
    std::optional<std::string> toString(const JsonObject& j, const char* key);
    JsonDocument fromString(std::string_view jsonStr);
    JsonDocument fromFile(std::string_view path);
    constexpr int defaultFloatPrecision = 5;
}

export template <typename T>
JsonObject serialize(const T&, Json::MemoryPoolAllocator<>&) { return {}; }

export template <typename T> 
T deserialize(const JsonObject&) { return T{}; }