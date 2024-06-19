module;
export module Engine.AssetManager;
import std;
import <windows.h>;
import <External/Json/json.hpp>;

namespace AssetManager
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

    export const nlohmann::json& getEngineConfig()
    {
        static nlohmann::json config = []
        {
            nlohmann::json j;
            std::string path = getEngineSourceRoot() + "Config.json";
            std::ifstream i{path};
            i >> j;
            return j;
        }();

        return config;
    }

    export const std::string& getContentRoot()
    {
        static const std::string path = []
        {
            std::string rootRelative = getEngineConfig()["contentRoot"];
            std::filesystem::path completePath{getExecutableRoot() + "../../" + rootRelative};
            return canonical(completePath).generic_string() + "/";
        }();
        return path;
    }
}
