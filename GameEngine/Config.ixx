module;
//#pragma warning(disable : 5105)
#include <windows.h>
export module Engine.Config;
import Engine.Guid;
import Math;
import std;
import <External/Json/json.hpp>;

export struct ApplicationSettings
{
    ivec2 resolution{800, 600};
    float targetFps{120.f};
    int numThreads{6};
};

export namespace glm
{
    template <int Size, typename T>
    // ReSharper disable once CppInconsistentNaming
    void from_json(const nlohmann::json& j, vec<Size, T>& val)
    {
        std::array<T, Size> arr = j;
        for (int i = 0; i < Size; ++i)
            val[i] = arr[i];
    }

    // ReSharper disable once CppInconsistentNaming
    void to_json(nlohmann::json& j, const ivec2& val)
    {
        j = nlohmann::json{val.x, val.y};
    }
}

export using Json = nlohmann::json;

export template <typename T>
T deserialize(const Json& serializedData) { return T{}; }

namespace Config
{
    export const std::string& getExecutableRoot()
    {
        static const std::string exeRoot = []
        {
            std::string buffer;
            buffer.resize(MAX_PATH);
            GetModuleFileName(nullptr, buffer.data(), MAX_PATH);
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
    
    export const Json& getEngineConfig()
    {
        static Json config = []
        {
            Json j;
            std::string path = getEngineSourceRoot() + "Config.json";
            std::ifstream i{path};
            i >> j;
            return j;
        }();

        return config;
    }

    export const ApplicationSettings& getApplicationSettings()
    {
        static const ApplicationSettings settings = []
        {
            ApplicationSettings ret;
            const Json& json = getEngineConfig();
            if (auto it = json.find("maxFps"); it != json.end())
            {
                ret.targetFps = *it;
            }
            if (auto it = json.find("resolution"); it != json.end())
            {
                ret.resolution = *it;
            }
            return ret;
        }();

        return settings;
    }

    export const std::string& getContentRoot()
    {
        static const std::string path = []
        {
            std::string rootRelative = *getEngineConfig().find("contentRoot");
            std::filesystem::path completePath{getExecutableRoot() + "../../" + rootRelative};
            return canonical(completePath).generic_string() + "/";
        }();
        return path;
    }
}
