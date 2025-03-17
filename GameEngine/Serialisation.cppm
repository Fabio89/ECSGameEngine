export module Engine:Serialisation;
import :Core;
import :Math;
import Wrapper.RapidJson;

export using ::JsonDocument;
export using ::JsonObject;

export namespace Json
{
    std::optional<vec2> parseVec2(const JsonObject& j, const char* key);
    std::optional<vec3> parseVec3(const JsonObject& j, const char* key);
    std::optional<std::string> parseString(const JsonObject& j, const char* key);
    JsonDocument fromString(std::string_view jsonStr);
    JsonDocument fromFile(std::string_view path);
}

export template <typename T>
T deserialize(const JsonObject& serializedData) { return T{}; }