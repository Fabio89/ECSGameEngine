module;

export module Engine.Config;
import Engine.AssetManager;
import std;
import <glm/glm.hpp>;
import <External/Json/json.hpp>;

export struct ApplicationSettings
{
    glm::ivec2 resolution{800, 600};
    float targetFps{120.f};
    int numThreads{6};
};

namespace Config
{
    export const nlohmann::json& getEngineConfig()
    {
        static nlohmann::json config = []
        {
            nlohmann::json j;
            std::string path = AssetManager::getEngineSourceRoot() + "Config.json";
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
            const nlohmann::json& json = getEngineConfig();
            if (auto propName = "maxFps"; json.contains(propName))
            {
                ret.targetFps = json[propName];
            }
            if (auto propName = "resolution"; json.contains(propName))
            {
                std::array<int, 2> res = json[propName];
                ret.resolution = {res[0], res[1]};
            }
            return ret;
        }();

        return settings;
    }

    export const std::string& getContentRoot()
    {
        static const std::string path = []
        {
            std::string rootRelative = getEngineConfig()["contentRoot"];
            std::filesystem::path completePath{AssetManager::getExecutableRoot() + "../../" + rootRelative};
            return canonical(completePath).generic_string() + "/";
        }();
        return path;
    }
}
