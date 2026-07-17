export module Assets.Mesh;
import Core;
import Math;
import Render.Vulkan;

//------------------------------------------------------------------------------------------------------------------------
// Vertex
//------------------------------------------------------------------------------------------------------------------------

export struct Vertex
{
    Vec3 pos{};
    Vec2 uv{};
};

export struct LineVertex
{
    Vec3 pos{};
    Vec3 colour{1, 1, 1};
};

bool operator==(const Vertex& a, const Vertex& b);

template <>
struct std::hash<Vertex>;

//------------------------------------------------------------------------------------------------------------------------
// Mesh
//------------------------------------------------------------------------------------------------------------------------

export struct MeshData
{
    std::vector<Vertex> vertices{};
    std::vector<UInt32> indices{};

    static constexpr auto indexType{vk::IndexType::eUint32};
};

template<>
constexpr std::string_view getTypeName<MeshData>() { return "Mesh"; }