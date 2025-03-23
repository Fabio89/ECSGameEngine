module Serialization;
import Log;
import Math;

namespace Json
{
    template <typename T>
    JsonObject fromVec(const T& v, MemoryPoolAllocator<>& allocator)
    {
        JsonObject json{kArrayType};
        json.Reserve(T::length(), allocator);
        for (int i = 0; i < T::length(); ++i)
        {
            json.PushBack(v[i], allocator);
        }

        return json;
    }

    template <typename T>
    std::optional<T> toVec(const JsonObject& j, const char* key)
    {
        const auto it = j.FindMember(key);
        if (it == j.MemberEnd())
            return std::nullopt;

        static constexpr SizeType length{T::length()};
        const auto& array = it->value.GetArray();
        if (array.Size() < length)
            return std::nullopt;

        T v;
        for (int i = 0; i < length; ++i)
        {
            v[i] = array[i].GetFloat();
        }
        return v;
    }
}

[[nodiscard]]
JsonObject Json::fromVec2(Vec2 v, MemoryPoolAllocator<>& allocator)
{
    return fromVec(v, allocator);
}

[[nodiscard]]
std::optional<Vec2> Json::toVec2(const JsonObject& j, const char* key)
{
    return toVec<Vec2>(j, key);
}

[[nodiscard]]
JsonObject Json::fromVec3(Vec3 v, MemoryPoolAllocator<>& allocator)
{
    return fromVec(v, allocator);
}

[[nodiscard]]
std::optional<Vec3> Json::toVec3(const JsonObject& j, const char* key)
{
    return toVec<Vec3>(j, key);
}

[[nodiscard]]
JsonObject Json::fromQuat(Quat q, MemoryPoolAllocator<>& allocator)
{
    return fromVec(q, allocator);
}

[[nodiscard]]
std::optional<Quat> Json::toQuat(const JsonObject& j, const char* key)
{
    return toVec<Quat>(j, key);
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

void Json::toFile(const JsonDocument& document, std::string_view path)
{
    std::ofstream ofs{path.data()};
    if (!ofs.is_open())
    {
        report(std::format("Failed to open file: '{}'", path));
        return;
    }

    OStreamWrapper osw{ofs};
    PrettyWriter writer{osw};
    writer.SetMaxDecimalPlaces(defaultFloatPrecision);
    document.Accept(writer);
}
