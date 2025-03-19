module Engine:Serialization;
import :Serialization;

[[nodiscard]]
JsonObject Json::fromVec2(vec3 v, MemoryPoolAllocator<> allocator)
{
    JsonObject json{kArrayType};
    json.PushBack(v.x, allocator);
    json.PushBack(v.y, allocator);
    return json;
}

[[nodiscard]]
std::optional<vec2> Json::toVec2(const JsonObject& j, const char* key)
{
    const auto it = j.FindMember(key);
    if (it == j.MemberEnd())
        return std::nullopt;

    const auto& array = it->value.GetArray();
    if (array.Size() < 2)
        return std::nullopt;

    return vec2{array[0].GetFloat(), array[1].GetFloat()};
}

[[nodiscard]]
JsonObject Json::fromVec3(vec3 v, MemoryPoolAllocator<> allocator)
{
    JsonObject json{kArrayType};
    json.PushBack(v.x, allocator);
    json.PushBack(v.y, allocator);
    json.PushBack(v.z, allocator);
    return json;
}

[[nodiscard]]
std::optional<vec3> Json::toVec3(const JsonObject& j, const char* key)
{
    const auto it = j.FindMember(key);
    if (it == j.MemberEnd())
        return std::nullopt;

    const auto& array = it->value.GetArray();
    if (array.Size() < 3)
        return std::nullopt;

    return vec3{array[0].GetFloat(), array[1].GetFloat(), array[2].GetFloat()};
}

[[nodiscard]]
std::optional<std::string> Json::toString(const JsonObject& j, const char* key)
{
    const auto it = j.FindMember(key);
    if (it == j.MemberEnd())
        return std::nullopt;

    return std::string{it->value.GetString()};
}

[[nodiscard]]
JsonDocument Json::fromString(std::string_view jsonStr)
{
    JsonDocument doc;
    doc.Parse(jsonStr.data());
    return doc;
}

[[nodiscard]]
JsonDocument Json::fromFile(std::string_view path)
{
    JsonDocument doc;
    if (std::ifstream ifs{path.data()}; check(!ifs.bad(), std::format("Failed to open file: '{}'", path)))
    {
        IStreamWrapper isw{ifs};
        doc.ParseStream(isw);
    }
    return doc;
}