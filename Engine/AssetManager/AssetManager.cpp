module AssetManager;

namespace
{
    std::filesystem::path contentRoot;
    std::unordered_map<TypeId, std::unique_ptr<AssetLoaderBase>> loaders;
    std::unordered_map<Guid, AssetEntry> assetEntries;
    std::unordered_map<std::string, TypeId> assetTypeNameToId;
}

AssetLoaderBase* AssetManager::getLoader(TypeId assetTypeId)
{
    auto it = loaders.find(assetTypeId);
    return it != loaders.end() ? it->second.get() : nullptr;
}

AssetEntry* AssetManager::getEntry(const Guid& assetId)
{
    auto it = assetEntries.find(assetId);
    return it != assetEntries.end() ? &it->second : nullptr;
}

Guid AssetManager::addEntry(std::string_view name, TypeId type, std::any&& data)
{
    Guid id = Guid::createRandom();
    assetEntries.emplace(id, AssetEntry{
                         .guid = id,
                         .name = std::string{name},
                         .type = type,
                         .data = std::move(data),
                         .path = {},
                         .loaded = true
                     });
    return id;
}

void AssetManager::loadDatabase(const std::filesystem::path& path)
{
    assetEntries.clear();

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
            std::filesystem::path assetPath = element.FindMember("path")->value.GetString();

            const TypeId typeId = assetTypeNameToId.at(typeName);

            assetEntries.emplace(id, AssetEntry{
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

void AssetManager::setContentRoot(std::filesystem::path path)
{
    contentRoot = std::move(path);
}

void AssetManager::registerLoader(TypeId assetTypeId, std::string assetTypeName, std::unique_ptr<AssetLoaderBase> loader)
{
    loaders[assetTypeId] = std::move(loader);
    assetTypeNameToId[std::move(assetTypeName)] = assetTypeId;
}

const std::filesystem::path& AssetManager::getContentRoot()
{
    return contentRoot;
}
