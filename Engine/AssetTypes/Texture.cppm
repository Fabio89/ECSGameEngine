export module Assets.Texture;
import Core;
import Serialization.Json;

export using TextureId = std::size_t;

export struct TextureData
{
    std::filesystem::path path{};
};

template<>
constexpr std::string_view getTypeName<TextureData>() { return "Texture"; }

template <>
TextureData deserialize(const JsonObject& serializedData)
{
    TextureData data;
    if (auto it = serializedData.FindMember("path"); it != serializedData.MemberEnd())
    {
        data.path.assign(it->value.GetString());
    }
    return data;
}