export module Assets.Texture;
import Core;

export struct TextureData
{
    std::filesystem::path path{};
};

template<>
constexpr std::string_view getTypeName<TextureData>() { return "Texture"; }