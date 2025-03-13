module Engine:AssetManager;

AssetBase::AssetBase(const JsonObject& serializedData)
{
    if (const auto id = parseString(serializedData, "id"))
    {
        m_id = Guid::createFromString(*id);
    }
    else
    {
        m_id = Guid::createRandom();
    }
}