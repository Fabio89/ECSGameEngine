export module Engine:Project;
import :Core;
import :Serialization;
import :World;
import Wrapper.Windows;

class Project
{
public:
    static void open(std::string path, World& world);
    static std::string getContentRoot();

private:
    inline static std::string currentProjectPath;
};

void Project::open(std::string path, World& world)
{
    currentProjectPath = std::move(path);
    world.loadScene(currentProjectPath);
}

std::string Project::getContentRoot()
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