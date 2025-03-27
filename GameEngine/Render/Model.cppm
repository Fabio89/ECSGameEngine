export module Render.Model;
import AssetManager;
import Core;
import Math;
import Serialization;
import Wrapper.Vulkan;

using InstanceId = size_t;

//------------------------------------------------------------------------------------------------------------------------
// Vertex
//------------------------------------------------------------------------------------------------------------------------

export struct Vertex
{
    Vec3 pos;
    Vec2 uv;
};

export struct LineVertex
{
    Vec3 pos;
    Vec3 colour{1, 1, 1};
};

bool operator==(const Vertex& a, const Vertex& b);

template <>
struct std::hash<Vertex> { size_t operator()(Vertex const& vertex) const noexcept; };

//------------------------------------------------------------------------------------------------------------------------
// Texture
//------------------------------------------------------------------------------------------------------------------------

export using TextureId = InstanceId;

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

export using MeshId = InstanceId;

export struct MeshData
{
    std::vector<Vertex> vertices;
    std::vector<UInt32> indices;

    static constexpr auto indexType{vk::IndexType::eUint32};
};

export using MeshAsset = Asset<MeshData>;

template <>
MeshData deserialize(const JsonObject& serializedData);