module Engine.AssetManager;

AssetBase::AssetBase(const Json& serializedData)
{
    if (auto it = serializedData.find("id"); it != serializedData.end())
    {
        m_id = Guid{*it};
    }
    else
    {
        m_id = {};
    }
}