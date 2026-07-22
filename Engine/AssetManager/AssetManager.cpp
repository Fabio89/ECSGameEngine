module AssetManager;

namespace
{
    std::unordered_map<TypeId, std::unique_ptr<AssetLoaderBase>> loaders;
    std::unordered_map<std::string, TypeId> assetTypeNameToId;
}

AssetLoaderBase* AssetManager::getLoader(TypeId assetTypeId)
{
    auto it = loaders.find(assetTypeId);
    return it != loaders.end() ? it->second.get() : nullptr;
}

AssetEntry* AssetManager::getEntry(const Guid& assetId)
{
    auto it = m_assetEntries.find(assetId);
    return it != m_assetEntries.end() ? &it->second : nullptr;
}

Guid AssetManager::addEntry(std::string_view name, TypeId type, std::any&& data)
{
    Guid id = Guid::createRandom();
    m_assetEntries.emplace(id, AssetEntry{
        .guid = id,
        .name = std::string{name},
        .type = type,
        .data = std::move(data),
        .path = {},
        .loaded = true
    });
    return id;
}

Guid AssetManager::addEntry(std::string_view name, TypeId type, AssetPath&& path)
{
    Guid id = Guid::createRandom();
    m_assetEntries.emplace(id, AssetEntry{
        .guid = id,
        .name = std::string{name},
        .type = type,
        .data = {},
        .path = std::move(path),
        .loaded = false
    });

    return id;
}

std::filesystem::path AssetManager::toAbsolutePath(const AssetPath& path) const
{
    if (auto it = m_mounts.find(path.mountId); it != m_mounts.end())
        return it->second.root / path.relativeToMount;
    return {};
}

AssetMountId AssetManager::mount(std::string name, std::filesystem::path path)
{
    const AssetMountId id{m_nextMountId++};
    m_mounts[id] = AssetMount{.id = id, .name = std::move(name), .root = std::move(path)};
    return id;
}

void AssetManager::unmount(AssetMountId mountId)
{
    m_mounts.erase(mountId);

    for (auto it = m_assetEntries.begin(); it != m_assetEntries.end(); )
    {
        if (it->second.path.mountId == mountId)
            it = m_assetEntries.erase(it);
        else
            ++it;
    }
}

void AssetManager::loadDatabase(AssetMountId mountId, const std::filesystem::path& path)
{
    if (!mountId.isValid() || !m_mounts.contains(mountId))
    {
        report(std::format("Tried to load asset database '{}' without providing a valid mount point!", path.string()));
    }

    if (!std::filesystem::exists(path))
    {
        JsonDocument document;
        document.AddMember("assets", JsonObject{Json::kArrayType}, document.GetAllocator());
        Json::toFile(document, path);
        return;
    }

    const JsonObject& json = Json::fromFile(path);

    if (const auto assets = json.FindMember("assets"); assets != json.MemberEnd())
    {
        for (const JsonObject& element : assets->value.GetArray())
        {
            Guid id = Guid::createFromString(element.FindMember("id")->value.GetString());
            std::string name = element.FindMember("name")->value.GetString();
            std::string typeName = element.FindMember("type")->value.GetString();
            AssetPath assetPath{.mountId = mountId, .relativeToMount = element.FindMember("path")->value.GetString()};

            const TypeId typeId = assetTypeNameToId.at(typeName);

            m_assetEntries.emplace(id, AssetEntry{
                .guid = id,
                .name = std::move(name),
                .type = typeId,
                .data = {},
                .path = std::move(assetPath),
                .loaded = false
            });
        }
    }
}

std::filesystem::path AssetManager::getContentRoot(AssetMountId mountId) const
{
    if (auto it = m_mounts.find(mountId); it != m_mounts.end())
        return it->second.root;
    return {};
}

std::vector<std::filesystem::path> AssetManager::listDirectories(AssetMountId mountId, const std::filesystem::path& relative) const
{
    std::vector<std::filesystem::path> result;

    const std::filesystem::path absolutePath = toAbsolutePath({mountId, relative});
    if (!std::filesystem::exists(absolutePath))
        return result;

    std::error_code ec;
    for (std::filesystem::directory_iterator it(absolutePath, ec), end; it != end && !ec; it.increment(ec))
    {
        if (it->is_directory(ec))
            result.push_back(std::filesystem::canonical(absolutePath / it->path().filename()));
    }
    return result;
}

std::vector<std::filesystem::path> AssetManager::listFiles(AssetMountId mountId, const std::filesystem::path& relative) const
{
    std::vector<std::filesystem::path> result;

    const std::filesystem::path absolutePath = toAbsolutePath({mountId, relative});
    if (!std::filesystem::exists(absolutePath))
        return result;

    std::error_code ec;
    for (std::filesystem::directory_iterator it(absolutePath, ec), end; it != end && !ec; it.increment(ec))
    {
        result.push_back(std::filesystem::canonical(absolutePath / it->path().filename()));
    }
    return result;
}

void AssetManager::registerLoader(TypeId assetTypeId, std::string assetTypeName, std::unique_ptr<AssetLoaderBase> loader)
{
    loaders[assetTypeId] = std::move(loader);
    assetTypeNameToId[std::move(assetTypeName)] = assetTypeId;
}
