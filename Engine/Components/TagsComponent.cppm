export module Component.Tags;
import Core;
import Properties;
import Serialization.Json;

export namespace Tag
{
    const std::string editorOnly = "EditorOnly";
}

export struct TagsComponent
{
    std::vector<std::string> tags{};
};

template<>
constexpr std::string_view getTypeName<TagsComponent>() { return "TagsComponent"; }

template<>
struct TypeProperties<TagsComponent>
{
    static constexpr std::tuple list{
        makeProperty("tags", &TagsComponent::tags)
    };
};

template<>
JsonObject serialize(const TagsComponent &component, Json::MemoryPoolAllocator<> &allocator)
{
    JsonObject json{Json::kArrayType};
    json.Reserve(component.tags.size(), allocator);
    for (const auto &tag: component.tags)
    {
        json.PushBack(JsonObject{tag.data(), allocator}, allocator);
    }
    return json;
}

template<>
TagsComponent deserialize(const JsonObject &data)
{
    return {};
}
