export module Engine:Config;
import :Core;
import :Math;
import Wrapper.RapidJson;
import Wrapper.Windows;

export struct ApplicationSettings
{
    ivec2 resolution{800, 600};
    float targetFps{144.f};
    int numThreads{6};
};

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
            const auto moduleName = Wrapper_Windows:: getModuleFileName();
            const auto pos = moduleName.find_last_of("\\/") + 1;
            return std::filesystem::canonical(moduleName.substr(0, pos)).generic_string() + "/";
        }();
        return exeRoot;
    }

    export const std::string& getEngineSourceRoot()
    {
        static const std::string path = std::filesystem::canonical(getExecutableRoot() + "../../GameEngine").
            generic_string() + "/";
        return path;
    }

    export [[nodiscard]] JsonDocument parseString(std::string_view jsonStr)
    {
        JsonDocument doc;
        doc.Parse(jsonStr.data());
        return doc;
    }
    
    export [[nodiscard]] JsonDocument parseFile(std::string_view path)
    {
        std::ifstream ifs{path.data()};
        check(ifs.is_open(), std::format("Failed to open file: {}", path));

        JsonDocument doc;
        IStreamWrapper isw{ifs};
        doc.ParseStream(isw);
        return doc;
    }

    export [[nodiscard]] JsonDocument getEngineConfig()
    {
        return parseFile(getExecutableRoot() + "Config.json");
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
