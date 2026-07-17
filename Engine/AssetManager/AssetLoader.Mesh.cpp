module;

#include <algorithm>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

module AssetLoader.Mesh;
import Core;
import Geometry;
import Math;
import Render.Vulkan;
import Serialization.Json;

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
    Size2D size;
    int texChannels;
    stbi_uc* pixels = stbi_load(path.c_str(), &size.width, &size.height, &texChannels, STBI_rgb_alpha);

    if (!check(pixels, std::format("Failed to load texture image: {}", path.string())))
    {
        static const TextureData invalidTexture{};
        return invalidTexture;
    }

    const TextureData data
    {
        .size = size,
        .format = vk::Format::eR8G8B8A8Srgb,
        .pixels = std::vector(pixels, pixels + size.width * size.height * 4)
    };

    stbi_image_free(pixels);

    return data;
}
