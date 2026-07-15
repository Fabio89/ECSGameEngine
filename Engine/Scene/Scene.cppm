export module Scene;
import Core;
import Serialization.Json;
import World;

export class Scene
{
public:
    Scene(World& world, const std::filesystem::path& path);

    void deserialize(World& world, const std::filesystem::path& path);
    void serialize(const World& world, const std::filesystem::path& path);
    void clear(World& world);

private:
    std::vector<Entity> m_entities;
};
