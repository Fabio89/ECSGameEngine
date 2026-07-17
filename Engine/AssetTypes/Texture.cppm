export module Assets.Texture;
import Core;
import Geometry;
import Render.Vulkan;

export struct TextureData
{
    Size2D size;
    vk::Format format{vk::Format::eR8G8B8A8Srgb};
    std::vector<UInt8> pixels;
};

template<>
constexpr std::string_view getTypeName<TextureData>() { return "Texture"; }