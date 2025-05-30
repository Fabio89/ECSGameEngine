export module Project;
import Core;
import Serialization;
import World;
import Wrapper.Windows;

export class Project
{
public:
    static void open(std::string path, World& world);
    static void saveToCurrent(const World& world);
    static std::string getContentRoot();

private:
    inline static std::string currentProjectPath;
};

export void Project::open(std::string path, World& world)
{
    currentProjectPath = std::move(path);
    world.loadScene(currentProjectPath);
}

export void Project::saveToCurrent(const World& world)
{
    if (!check(!currentProjectPath.empty(), "Can't save current project as none is open!"))
        return;

    JsonDocument doc = Json::fromFile(currentProjectPath);
    if (auto it = doc.FindMember("entities"); it != doc.MemberEnd())
    {
        JsonObject& scene = it->value = world.serializeScene(doc.GetAllocator());
        it->value = std::move(scene.FindMember("entities")->value);
    }
    Json::toFile(doc, currentProjectPath);
}

export std::string Project::getContentRoot()
{
    JsonDocument doc = Json::fromFile(currentProjectPath);
    if (doc.IsObject())
    {
        if (auto contentRootIt = doc.FindMember("contentRoot"); contentRootIt != doc.MemberEnd())
        {
            std::filesystem::path completePath{currentProjectPath};
            completePath.replace_filename(contentRootIt->value.GetString());
            return canonical(completePath).generic_string() + "/";
        }
    }
    report("No content root specified in the project file! Using default 'Content'", ErrorType::Warning);
    return "Content";
}
