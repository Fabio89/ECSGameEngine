module;

//#pragma warning(disable : 5105)
#include <windows.h>
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

export module Engine:Config;
import :Math;

export struct ApplicationSettings
{
    ivec2 resolution{800, 600};
    float targetFps{120.f};
    int numThreads{6};
};

export using JsonDocument = rapidjson::Document;
export using JsonObject = rapidjson::Value;

export std::optional<vec2> parseVec2(const JsonObject& j, const char* key)
{
    const auto it = j.FindMember(key);
    if (it == j.MemberEnd())
        return std::nullopt;

    const auto& array = it->value.GetArray();
    if (array.Size() < 2)
        return std::nullopt;

    return vec2{array[0].GetFloat(), array[1].GetFloat()};
}

export std::optional<vec3> parseVec3(const JsonObject& j, const char* key)
{
    const auto it = j.FindMember(key);
    if (it == j.MemberEnd())
        return std::nullopt;

    const auto& array = it->value.GetArray();
    if (array.Size() < 3)
        return std::nullopt;

    return vec3{array[0].GetFloat(), array[1].GetFloat(), array[2].GetFloat()};
}

export std::optional<std::string> parseString(const JsonObject& j, const char* key)
{
    const auto it = j.FindMember(key);
    if (it == j.MemberEnd())
        return std::nullopt;

    return std::string{it->value.GetString()};
}

export template <typename T>
T deserialize(const JsonObject& serializedData) { return T{}; }

namespace Config
{
    export const std::string& getExecutableRoot()
    {
        static const std::string exeRoot = []
        {
            std::string buffer;
            buffer.resize(MAX_PATH);
            GetModuleFileNameA(nullptr, buffer.data(), MAX_PATH);
            auto pos = buffer.find_last_of("\\/") + 1;
            return std::filesystem::canonical(buffer.substr(0, pos)).generic_string() + "/";
        }();
        return exeRoot;
    }

    export const std::string& getEngineSourceRoot()
    {
        static const std::string path = std::filesystem::canonical(getExecutableRoot() + "../../GameEngine").
            generic_string() + "/";
        return path;
    }

    export const JsonDocument& getEngineConfig()
    {
        static JsonDocument config = []
        {
            JsonDocument j;
            std::string path = getEngineSourceRoot() + "Config.json";
            std::ifstream ifs{path};
            rapidjson::IStreamWrapper isw{ifs};
            j.ParseStream(isw);
            return j;
        }();

        return config;
    }

    export const ApplicationSettings& getApplicationSettings()
    {
        static const ApplicationSettings settings = []
        {
            ApplicationSettings ret;
            const JsonDocument& json = getEngineConfig();

            if (auto it = json.FindMember("maxFps"); it != json.MemberEnd())
            {
                ret.targetFps = it->value.GetFloat();
            }
            if (auto it = json.FindMember("resolution"); it != json.MemberEnd())
            {
                ret.resolution = ivec2{it->value.GetArray()[0].GetInt(), it->value.GetArray()[1].GetInt()};
            }
            return ret;
        }();

        return settings;
    }

    export const std::string& getContentRoot()
    {
        static const std::string path = []
        {
            std::string rootRelative = getEngineConfig().FindMember("contentRoot")->value.GetString();
            std::filesystem::path completePath{getExecutableRoot() + "../../" + rootRelative};
            return canonical(completePath).generic_string() + "/";
        }();
        return path;
    }
}
