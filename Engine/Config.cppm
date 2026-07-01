export module Engine.Config;
import Core;
import Geometry;
import Serialization.Toml;

namespace
{
    const std::filesystem::path defaultConfigPath{"Engine/Config/DefaultEngine.toml"};
    const std::filesystem::path configPath{"Engine/Config/Engine.toml"};
}

export namespace Engine
{
    struct Config
    {
        struct Window
        {
            Size2D size{800, 600};
            bool maximized{false};
        } window;

        float simulationHz{120.0f};
    };

    [[nodiscard]]
    Config loadConfig()
    {
        if (!std::filesystem::exists(configPath))
        {
            const auto defaultConfig = Toml::fromFile(defaultConfigPath);
            Toml::toFile(defaultConfig, configPath);
        }

        const auto table = Toml::fromFile(configPath);

        Config config
        {
            .window =
            {
                .size = {table["window"]["width"].value_or(config.window.size.width), table["window"]["height"].value_or(config.window.size.height)},
                .maximized = table["window"]["maximized"].value_or(config.window.maximized),
            },
            .simulationHz = table["time"]["simulation_hz"].value_or(config.simulationHz)
        };

        return config;
    }
}
