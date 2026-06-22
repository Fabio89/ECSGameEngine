export module Component.Tags;
import Core;
import Serialization;

export namespace Tag
{
    constexpr std::string_view notEditable = "NotEditable";
}

export struct TagsComponent
{
    std::vector<std::string_view> tags;
};

template <>
struct TypeTraits<TagsComponent>
{
    static constexpr auto name = "TagsComponent";
};

template <>
JsonObject serialize(const TagsComponent& component, Json::MemoryPoolAllocator<>& allocator)
{
    JsonObject json{Json::kArrayType};
    json.Reserve(component.tags.size(), allocator);
    for (const auto& tag : component.tags)
    {
        json.PushBack(JsonObject{tag.data(), allocator}, allocator);
    }
    return json;
}

template <>
TagsComponent deserialize(const JsonObject& data)
{
    return {};
}
