export module Engine.ComponentRegistry;

import Engine.Config;
import Engine.Core;
import Engine.World;
import std;

namespace ComponentRegistry
{
    using ComponentCreateFunc = std::function<void(World&, Entity, const Json&)>;
    std::unordered_map<std::string, ComponentCreateFunc> g_registry;

    template <typename T>
    void createComponent(World& world, Entity entity, const Json& data)
    {
        world.addComponent<T>(entity, Deserialize<T>(data));
    }

    export template <typename T>
    void registerComponent()
    {
        std::string name{typeid(T).name()};

        const size_t endPos = name.find_last_not_of(' ');
        size_t startPos = name.find_last_of(' ', endPos);
        if (startPos == std::string::npos)
            startPos = 0;

        std::string cleanName = name.substr(startPos + 1, endPos - startPos);
        g_registry[cleanName] = createComponent<T>;
    };

    export ComponentCreateFunc getComponentCreateFunc(const std::string& typeName)
    {
        return g_registry[typeName];
    }
}
