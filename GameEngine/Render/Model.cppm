export module Engine:Render.Model;
import :AssetManager;
import :Render.Vulkan;
import :Serialization;

using IdType = size_t;

//------------------------------------------------------------------------------------------------------------------------
// Vertex
//------------------------------------------------------------------------------------------------------------------------

export struct Vertex
{
    vec3 pos;
    vec2 uv;
};

bool operator==(const Vertex& a, const Vertex& b);

template <>
struct std::hash<Vertex> { size_t operator()(Vertex const& vertex) const noexcept; };

//------------------------------------------------------------------------------------------------------------------------
// Texture
//------------------------------------------------------------------------------------------------------------------------

export using TextureId = IdType;

export struct TextureData
{
    std::string path;
};

export using TextureAsset = Asset<TextureData>;

template <>
TextureData deserialize(const JsonObject& serializedData);

//------------------------------------------------------------------------------------------------------------------------
// Mesh
//------------------------------------------------------------------------------------------------------------------------

export using MeshId = IdType;

export struct MeshData
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    static constexpr auto indexType{vk::IndexType::eUint32};
};

export using MeshAsset = Asset<MeshData>;

template <>
MeshData deserialize(const JsonObject& serializedData);