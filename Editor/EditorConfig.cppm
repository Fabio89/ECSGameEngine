export module Editor.Config;
import Core;
import Serialization.Toml;

namespace
{
    const std::filesystem::path defaultConfigPath{"Editor/Config/DefaultEditor.toml"};
    const std::filesystem::path configPath{"Editor/Config/Editor.toml"};
}

export struct EditorConfig
{
    std::filesystem::path lastProject;
};

export [[nodiscard]]
EditorConfig loadEditorConfig()
{
    if (!std::filesystem::exists(configPath))
    {
        std::filesystem::create_directories(configPath.parent_path());
        const auto defaultConfig = Toml::fromFile(defaultConfigPath);
        Toml::toFile(defaultConfig, configPath);
    }

    const auto table = Toml::fromFile(configPath);

    EditorConfig config
    {
        .lastProject = table["last_project"].value_or("")
    };

    return config;
}

export void saveEditorConfig(const EditorConfig& config)
{
    const Toml::Table table
    {
        {"last_project", config.lastProject.string()}
    };

    Toml::toFile(table, configPath);
}