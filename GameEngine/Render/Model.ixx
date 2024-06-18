module;

#define TINYOBJLOADER_IMPLEMENTATION
#include <External/MeshLoading/tiny_obj_loader.h>

export module Engine.Render.Core:Model;
import :Vulkan;
import std;
import <glm/glm.hpp>;

using IdType = size_t;
export using TextureId = IdType;
export using MeshId = IdType;

export struct Vertex
{
    glm::vec3 pos;
    glm::vec2 texCoordinates;
};

export struct MeshData
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    static constexpr auto indexType{vk::IndexType::eUint32};
};

export namespace ModelUtils
{
    MeshData loadModel(const char* path)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!LoadObj(&attrib, &shapes, &materials, &warn, &err, path))
        {
            throw std::runtime_error(warn + err);
        }

        const size_t vertexCount = std::reduce(shapes.begin(), shapes.end(), (size_t)0,
                                                   [](size_t a, auto&& b) { return a + b.mesh.indices.size(); });
        MeshData mesh;
        mesh.vertices.reserve(vertexCount);
        mesh.indices.reserve(vertexCount);

        for (const auto& shape : shapes)
        {
            for (const auto& index : shape.mesh.indices)
            {
                Vertex vertex
                {
                    .pos
                    {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                    },
                    .texCoordinates
                    {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        attrib.texcoords[2 * index.texcoord_index + 1]
                    },
                };
                mesh.vertices.emplace_back(std::move(vertex));
                mesh.indices.push_back(static_cast<uint32_t>(mesh.indices.size()));
            }
        }
        return mesh;
    }
}
