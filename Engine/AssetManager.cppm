export module AssetManager;
import Core;
import Guid;
import Serialization;

export using AssetTypeId = TypeId;

export class AssetBase
{
public:
    explicit AssetBase(const JsonObject& serializedData)
    {
        if (const auto id = Json::toString(serializedData, "id"))
        {
            m_id = Guid::createFromString(*id);
        }
        else
        {
            m_id = Guid::createRandom();
        }
    }

    const Guid& getGuid() const { return m_id; };

    virtual ~AssetBase() = default;

    virtual AssetTypeId getType() const = 0;

private:
    Guid m_id;
};

export template <typename T>
class Asset final : public AssetBase
{
public:
    inline static const AssetTypeId typeId = UniqueIdGenerator::TypeIdGenerator<T>::value;

    Asset(const JsonObject& serializedData)
        : AssetBase{serializedData},
          m_data{deserialize<T>(serializedData)}
    {
    }

    const T& getData() const { return m_data; }
    AssetTypeId getType() const override { return typeId; }

private:
    T m_data;
};

namespace AssetManager
{
    std::vector<std::unique_ptr<AssetBase>> loadedAssets;

    export template <typename T>
    const T* loadAsset(const JsonObject& serializedData)
    {
        return static_cast<const T*>(loadedAssets.emplace_back(std::make_unique<T>(serializedData)).get());
    }

    export template <typename T>
    const T* findAsset(const Guid& guid)
    {
        auto it = std::ranges::find_if(loadedAssets, [&](auto&& asset) { return asset->getGuid() == guid; });
        return it != loadedAssets.end() && (*it)->getType() == T::typeId ? static_cast<const T*>(it->get()) : nullptr;
    }
};
