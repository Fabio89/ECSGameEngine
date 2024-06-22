module;
export module Engine.Config;
import Engine.AssetManager;
import Engine.Guid;
import std;
import <External/Json/json.hpp>;
import <glm/glm.hpp>;

export struct ApplicationSettings
{
    glm::ivec2 resolution{800, 600};
    float targetFps{120.f};
    int numThreads{6};
};

export namespace glm
{
    template <int Size, typename T>
    void from_json(const nlohmann::json& j, vec<Size, T>& val)
    {
        std::array<T, Size> arr = j;
        for (int i = 0; i < Size; ++i)
            val[i] = arr[i];
    }

    void to_json(nlohmann::json& j, const ivec2& val)
    {
        j = nlohmann::json{val.x, val.y};
    }
}

export using Json = nlohmann::json;

namespace Config
{
    export const Json& getEngineConfig()
    {
        static Json config = []
        {
            Json j;
            std::string path = AssetManager::getEngineSourceRoot() + "Config.json";
            std::ifstream i{path};
            i >> j;
            return j;
        }();

        return config;
    }

    export const ApplicationSettings& getApplicationSettings()
    {
        static const ApplicationSettings settings = []
        {
            ApplicationSettings ret;
            const Json& json = getEngineConfig();
            if (auto it = json.find("maxFps"); it != json.end())
            {
                ret.targetFps = *it;
            }
            if (auto it = json.find("resolution"); it != json.end())
            {
                ret.resolution = *it;
            }
            return ret;
        }();

        return settings;
    }

    export const std::string& getContentRoot()
    {
        static const std::string path = []
        {
            std::string rootRelative = *getEngineConfig().find("contentRoot");
            std::filesystem::path completePath{AssetManager::getExecutableRoot() + "../../" + rootRelative};
            return canonical(completePath).generic_string() + "/";
        }();
        return path;
    }
}

export class AssetBase
{
public:
    AssetBase() = default;

    explicit AssetBase(const Json& serializedData);

    const Guid& getGuid() const { return m_id; };

    virtual ~AssetBase() = default;

private:
    Guid m_id;
};

export template <typename T>
T Deserialize(const Json& serializedData) { return T{}; }

export template <typename T>
class Asset : public AssetBase
{
public:
    using AssetBase::AssetBase;

    Asset(const Json& serializedData)
        : AssetBase{serializedData}
    , m_data{Deserialize<T>(serializedData)}
    {
    }

    const T& getData() const { return m_data; }

private:
    T m_data;
};
