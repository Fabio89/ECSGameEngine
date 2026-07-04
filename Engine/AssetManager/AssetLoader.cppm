export module AssetLoader;
import std;

export struct AssetLoaderBase
{
    virtual ~AssetLoaderBase() = default;
};

export template<typename T>
struct AssetLoader : AssetLoaderBase
{
    virtual T loadFromFile(const std::filesystem::path& path) = 0;
};