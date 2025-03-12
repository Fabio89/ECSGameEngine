module;

#define TINYOBJLOADER_IMPLEMENTATION
#include <External/MeshLoading/tiny_obj_loader.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <windows.h>

export module Engine.Render.Core:Model;
import :Vulkan;
import Engine.AssetManager;
import Engine.Config;
import Math;
import std;

using IdType = size_t;
export using TextureId = IdType;
export using MeshId = IdType;
export using MeshGuid = GUID;

export struct Vertex
{
    vec3 pos;
    vec2 uv;
};

bool operator==(const Vertex& a, const Vertex& b)
{
    return a.pos == b.pos && a.uv == b.uv;
}

template <>
struct std::hash<Vertex>
{
    size_t operator()(Vertex const& vertex) const noexcept
    {
        return ((hash_vector(vertex.pos) ^
            (hash_vector(vertex.uv) << 1)) >> 1);
    }
};

export struct TextureData
{
    std::string path;
};

template <>
TextureData deserialize(const Json& serializedData)
{
    TextureData data;
    if (auto it = serializedData.find("path"); it != serializedData.end())
    {
        data.path = *it;
    }
    return data;
}

export using TextureAsset = Asset<TextureData>;

export struct MeshData
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    static constexpr auto indexType{vk::IndexType::eUint32};
};

export using MeshAsset = Asset<MeshData>;

template <>
MeshData deserialize(const Json& serializedData);

export namespace ModelUtils
{
    MeshData loadModel(const char* path)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!LoadObj(&attrib, &shapes, &materials, &warn, &err, (Config::getContentRoot() + path).c_str()))
        {
            throw std::runtime_error(warn + err);
        }

        const size_t vertexCount = std::reduce(shapes.begin(), shapes.end(), size_t{0},
                                               [](size_t a, auto&& b) { return a + b.mesh.indices.size(); });
        MeshData mesh;
        mesh.vertices.reserve(vertexCount);
        mesh.indices.reserve(vertexCount);

        std::unordered_map<Vertex, uint32_t> uniqueVertices;

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
                    uniqueVertices[vertex] = static_cast<uint32_t>(mesh.vertices.size());
                    mesh.vertices.push_back(vertex);
                }

                mesh.indices.push_back(uniqueVertices[vertex]);
            }
        }
        return mesh;
    }
}

template <>
MeshData deserialize(const Json& serializedData)
{
    MeshData data;
    if (auto it = serializedData.find("path"); it != serializedData.end())
    {
        data = ModelUtils::loadModel(std::string{*it}.c_str());
    }
    else if (auto verticesIt = serializedData.find("vertices"), indicesIt = serializedData.find("indices"); verticesIt != serializedData.end() && indicesIt != serializedData.
        end())
    {
        for (const auto& verticesJson : *verticesIt)
        {
            auto& [pos, uv] = data.vertices.emplace_back();
            pos = *verticesJson.find("position");
            uv = *verticesJson.find("uv");
        }
        for (const auto& indicesJson : *indicesIt)
        {
            data.indices.emplace_back(indicesJson);
        }
    }
    return data;
}
