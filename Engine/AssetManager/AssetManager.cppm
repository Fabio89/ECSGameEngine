export module AssetManager;
import AssetLoader;
import Core;
import Guid;
import Serialization.Json;

struct AssetEntry
{
    Guid guid;
    std::string name;
    TypeId type;
    std::any data;
    std::filesystem::path path;
    bool loaded{};
};

namespace AssetManager
{
    AssetLoaderBase* getLoader(TypeId assetTypeId);
    AssetEntry* getEntry(const Guid& assetId);
    Guid addEntry(std::string_view name, TypeId type, std::any&& data);
    void registerLoader(TypeId assetTypeId, std::string assetTypeName, std::unique_ptr<AssetLoaderBase> loader);

    template<typename T>
    bool ensureLoaded(AssetEntry& entry);

    const std::filesystem::path& getContentRoot();
}

namespace AssetManager
{
    export void loadDatabase(const std::filesystem::path& path);

    export template <typename T>
    const T* tryResolve(const Guid& assetGuid)
    {
        if (AssetEntry* entry = getEntry(assetGuid))
            if (ensureLoaded<T>(*entry))
                return std::any_cast<const T>(&entry->data);
        return nullptr;
    }

    export template<typename T>
    const T& resolve(const Guid& assetGuid)
    {
        if (const T* asset = tryResolve<T>(assetGuid))
            return *asset;

        static const T invalid{};
        report(std::format("Couldn't resolve asset {}", assetGuid.toString()));
        return invalid;
    }

    export template<typename T>
    Guid add(std::string_view name, T&& assetData)
    {
        return addEntry(name, getTypeId<T>(), std::move(assetData));
    }

    export void setContentRoot(std::filesystem::path path);

    export template <typename T>
    void registerLoader(std::unique_ptr<AssetLoaderBase> loader) { registerLoader(getTypeId<T>(), std::string{getTypeName<T>()}, std::move(loader)); }
};


template<typename T>
bool AssetManager::ensureLoaded(AssetEntry& entry)
{
    if (!entry.loaded)
    {
        if (AssetLoaderBase* loader = getLoader(getTypeId<T>()))
        {
            auto* typedLoader = static_cast<AssetLoader<T>*>(loader);
            entry.data = typedLoader->loadFromFile(getContentRoot() / entry.path);
            entry.loaded = true;
        }
        else
        {
            return false;
        }
    }
    return true;
}