module;

export module Engine:AssetManager;
import :Config;
import :Guid;
import std;

export class AssetBase
{
public:
    explicit AssetBase(const JsonObject& serializedData);

    const Guid& getGuid() const { return m_id; };

    virtual ~AssetBase() = default;

    virtual std::type_index getType() const = 0;

private:
    Guid m_id;
};

export template <typename T>
class Asset final : public AssetBase
{
public:
    Asset(const JsonObject& serializedData)
        : AssetBase{serializedData},
          m_data{deserialize<T>(serializedData)}
    {
    }

    const T& getData() const { return m_data; }
    std::type_index getType() const override { return typeid(Asset); }

private:
    T m_data;
};

namespace AssetManager
{
    std::vector<std::unique_ptr<AssetBase>> g_loadedAssets;

    export template <typename T>
    const T* loadAsset(const JsonObject& serializedData)
    {
        return static_cast<const T*>(g_loadedAssets.emplace_back(std::make_unique<T>(serializedData)).get());
    }

    export template <typename T>
    const T* findAsset(const Guid& guid)
    {
        auto it = std::ranges::find_if(g_loadedAssets, [&](auto&& asset) { return asset->getGuid() == guid; });
        return it != g_loadedAssets.end() && (*it)->getType() == typeid(T) ? static_cast<const T*>(it->get()) : nullptr;
    }
};
