export module Editor.Project;
import Core;
import Serialization.Toml;

export struct ProjectConfig
{
    std::filesystem::path contentRoot;
    std::filesystem::path assetDatabase;
    std::filesystem::path startupScene;
};

export [[nodiscard]]
ProjectConfig loadProjectConfig(const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path))
    {
        return {};
    }

    const auto table = Toml::fromFile(path);

    ProjectConfig config
    {
        .contentRoot = table["content_root"].value_or("Content"),
        .assetDatabase = table["asset_database"].value_or("asset_database.json"),
        .startupScene = table["startup_scene"].value_or("")
    };

    return config;
}