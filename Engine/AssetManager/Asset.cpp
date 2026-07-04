// module Asset;
//
// AssetBase::AssetBase(const JsonObject& serializedData) {
//     if (const auto id = Json::toString(serializedData, "id"))
//     {
//         m_id = Guid::createFromString(*id);
//     }
//     else
//     {
//         m_id = Guid::createRandom();
//     }
// }