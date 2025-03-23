module;
#define TINYOBJLOADER_IMPLEMENTATION
#include <External/MeshLoading/tiny_obj_loader.h>

module Render.Model;
import Project;

bool operator==(const Vertex& a, const Vertex& b)
{
    return a.pos == b.pos && a.uv == b.uv;
}

template <>
struct std::hash<Vertex>
{
    size_t operator()(Vertex const& vertex) const noexcept
    {
        size_t seed = hash_value(vertex.pos);
        hash_combine(seed, hash_value(vertex.uv));
        return seed;
    }
};

MeshData loadModel(const char* path)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!LoadObj(&attrib, &shapes, &materials, &warn, &err, (Project::getContentRoot() + path).c_str()))
    {
        report((warn + err).c_str(), ErrorType::Error);
        return {};
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

template <>
MeshData deserialize(const JsonObject& serializedData)
{
    MeshData data;
    if (auto it = serializedData.FindMember("path"); it != serializedData.MemberEnd())
    {
        data = loadModel(std::string{it->value.GetString()}.c_str());
    }
    else if (auto verticesJson = serializedData.FindMember("vertices"),
        indicesJson = serializedData.FindMember("indices");
        verticesJson != serializedData.MemberEnd() && indicesJson != serializedData.MemberEnd())
    {
        for (const auto& vertices = verticesJson->value.GetArray(); const JsonObject& vertex : vertices)
        {
            auto& [pos, uv] = data.vertices.emplace_back();
            pos = Json::toVec3(vertex, "position").value_or(Vec3{});
            uv = Json::toVec2(vertex,"uv").value_or(Vec2{});
        }

        for (const auto& indices = indicesJson->value.GetArray(); const JsonObject& index : indices)
        {
            data.indices.emplace_back(index.GetUint());
        }
    }
    return data;
}

template <>
TextureData deserialize(const JsonObject& serializedData)
{
    TextureData data;
    if (auto it = serializedData.FindMember("path"); it != serializedData.MemberEnd())
    {
        data.path = it->value.GetString();
    }
    return data;
}