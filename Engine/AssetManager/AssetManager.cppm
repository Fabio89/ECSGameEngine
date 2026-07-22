export module AssetManager;
export import Guid;
import AssetLoader;
import Core;
import Serialization.Json;

export struct AssetMountId : Id<struct AssetMountTag> {};

template<>
struct std::hash<AssetMountId>
{
    constexpr std::size_t operator()(const AssetMountId& id) const noexcept { return std::hash<Id<AssetMountTag>>{}(id); }
};

export struct AssetPath
{
    AssetMountId mountId;
    std::filesystem::path relativeToMount;
};

struct AssetMount
{
    AssetMountId id;
    std::string name;
    std::filesystem::path root;
};

struct AssetEntry
{
    Guid guid;
    std::string name;
    TypeId type;
    std::any data;
    AssetPath path;
    bool loaded{};
};

export class AssetManager
{
public:
    AssetMountId mount(std::string name, std::filesystem::path path);
    void unmount(AssetMountId mountId);

    void loadDatabase(AssetMountId mountId, const std::filesystem::path& path);

    template<typename T>
    const T* tryResolve(const Guid& assetGuid);

    template<typename T>
    const T& resolve(const Guid& assetGuid);

    template<typename T>
    Guid addFromData(std::string_view name, T&& assetData);

    template<typename T>
    Guid addFromFile(std::string_view name, AssetPath path);

    std::filesystem::path getContentRoot(AssetMountId mountId) const;;
    std::vector<std::filesystem::path> listDirectories(AssetMountId mountId, const std::filesystem::path& relative) const;
    std::vector<std::filesystem::path> listFiles(AssetMountId mountId, const std::filesystem::path& relative) const;

    template<typename T>
    static void registerLoader(std::unique_ptr<AssetLoaderBase> loader) { registerLoader(getTypeId<T>(), std::string{getTypeName<T>()}, std::move(loader)); }

private:
    static void registerLoader(TypeId assetTypeId, std::string assetTypeName, std::unique_ptr<AssetLoaderBase> loader);

    AssetLoaderBase* getLoader(TypeId assetTypeId);

    AssetEntry* getEntry(const Guid& assetId);

    Guid addEntry(std::string_view name, TypeId type, std::any&& data);

    Guid addEntry(std::string_view name, TypeId type, AssetPath&& path);

    template<typename T>
    bool ensureLoaded(AssetEntry& entry);

    std::filesystem::path toAbsolutePath(const AssetPath& path) const;

    std::unordered_map<Guid, AssetEntry> m_assetEntries;
    std::unordered_map<AssetMountId, AssetMount> m_mounts;
    AssetMountId::ValueType m_nextMountId{};
};

template<typename T>
const T* AssetManager::tryResolve(const Guid& assetGuid)
{
    if (AssetEntry* entry = getEntry(assetGuid))
        if (ensureLoaded<T>(*entry))
            return std::any_cast<const T>(&entry->data);
    return nullptr;
}

template<typename T>
const T& AssetManager::resolve(const Guid& assetGuid)
{
    if (const T* asset = tryResolve<T>(assetGuid))
        return *asset;

    static const T invalid{};
    report(std::format("Couldn't resolve asset {}", assetGuid.toString()));
    return invalid;
}

template<typename T>
Guid AssetManager::addFromData(std::string_view name, T&& assetData)
{
    return addEntry(name, getTypeId<T>(), std::move(assetData));
}

template<typename T>
Guid AssetManager::addFromFile(std::string_view name, AssetPath path)
{
    return addEntry(name, getTypeId<T>(), std::move(path));
}

template<typename T>
bool AssetManager::ensureLoaded(AssetEntry& entry)
{
    if (!entry.loaded)
    {
        if (AssetLoaderBase* loader = getLoader(getTypeId<T>()))
        {
            auto* typedLoader = static_cast<AssetLoader<T>*>(loader);
            entry.data = typedLoader->loadFromFile(toAbsolutePath(entry.path));
            entry.loaded = true;
        }
        else
        {
            return false;
        }
    }
    return true;
}