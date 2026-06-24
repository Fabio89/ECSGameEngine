export module Component.Tags;
import Core;
import Properties;
import Serialization;

export namespace Tag
{
    constexpr std::string notEditable = "NotEditable";
}

export struct TagsComponent
{
    std::vector<std::string> tags{};
};

template<>
struct TypeTraits<TagsComponent>
{
    static constexpr std::string_view name = "TagsComponent";
};

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
