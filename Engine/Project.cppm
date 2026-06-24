export module Project;
import Core;
import Serialization.Json;
import World;

export class Project
{
public:
    static void open(std::filesystem::path path, World& world);
    static void saveToCurrent(const World& world);
    static std::filesystem::path getContentRoot();

private:
    inline static std::filesystem::path m_currentProjectPath;
};

void Project::open(std::filesystem::path path, World& world)
{
    m_currentProjectPath = std::move(path);
    world.loadScene(m_currentProjectPath);
}

void Project::saveToCurrent(const World& world)
{
    if (!check(!m_currentProjectPath.empty(), "Can't save current project as none is open!"))
        return;

    JsonDocument doc = Json::fromFile(m_currentProjectPath);
    if (auto it = doc.FindMember("entities"); it != doc.MemberEnd())
    {
        JsonObject& scene = it->value = world.serializeScene(doc.GetAllocator());
        it->value = std::move(scene.FindMember("entities")->value);
    }
    Json::toFile(doc, m_currentProjectPath);
}

std::filesystem::path Project::getContentRoot()
{
    if (JsonDocument doc = Json::fromFile(m_currentProjectPath); doc.IsObject())
    {
        if (auto contentRootIt = doc.FindMember("contentRoot"); contentRootIt != doc.MemberEnd())
        {
            std::filesystem::path completePath{m_currentProjectPath};
            completePath.replace_filename(contentRootIt->value.GetString());
            return canonical(completePath);
        }
    }
    report("No content root specified in the project file! Using default 'Content'", ErrorType::Warning);
    return std::filesystem::path{"Content"};
}
