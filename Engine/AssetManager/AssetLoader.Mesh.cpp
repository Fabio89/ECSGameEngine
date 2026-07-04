module;

#include <algorithm>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

module AssetLoader.Mesh;
import Core;
import Math;

bool operator==(const Vertex& a, const Vertex& b)
{
    return a.pos == b.pos && a.uv == b.uv;
}

template<>
struct std::hash<Vertex>
{
    std::size_t operator()(Vertex const& vertex) const noexcept
    {
        std::size_t seed = hash_value(vertex.pos);
        hash_combine(seed, hash_value(vertex.uv));
        return seed;
    }
};

MeshData MeshAssetLoader::loadFromFile(const std::filesystem::path& path)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
    {
        report((warn + err).c_str(), ErrorType::Error);
        return {};
    }

    const std::size_t vertexCount =
            std::ranges::fold_left(
                shapes,
                std::size_t{0},
                [](std::size_t a, const tinyobj::shape_t& b) { return a + b.mesh.indices.size(); });

    MeshData mesh;
    mesh.vertices.reserve(vertexCount);
    mesh.indices.reserve(vertexCount);

    std::unordered_map<Vertex, UInt32> uniqueVertices;

    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            const Vertex vertex
            {
                .pos
                {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                },
                .uv =
                {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                }
            };

            if (!uniqueVertices.contains(vertex))
            {
                uniqueVertices[vertex] = static_cast<UInt32>(mesh.vertices.size());
                mesh.vertices.push_back(vertex);
            }

            mesh.indices.push_back(uniqueVertices[vertex]);
        }
    }
    return mesh;
}

TextureData TextureAssetLoader::loadFromFile(const std::filesystem::path& path)
{
    return TextureData{.path = path};
}
