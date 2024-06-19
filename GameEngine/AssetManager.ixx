module;

#pragma warning(disable : 5105)
#include <windows.h>

export module Engine.AssetManager;
import std;

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
}
