module Assets.Mesh;

template <>
MeshData deserialize(const JsonObject& serializedData)
{
    MeshData data;
    if (auto it = serializedData.FindMember("path"); it != serializedData.MemberEnd())
    {
        //data = loadModel(it->value.GetString());
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