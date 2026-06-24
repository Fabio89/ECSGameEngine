export module Component.Hierarchy;
import Component.PersistentId;
import Core;
import Guid;
import Properties;
import Serialization;
import World;

export struct HierarchyComponent
{
    Entity parent{invalidId()};
    Entity firstChild{invalidId()};
    Entity nextSibling{invalidId()};
    Entity previousSibling{invalidId()};
};

template<>
struct TypeTraits<HierarchyComponent>
{
    static constexpr std::string_view name = "HierarchyComponent";
};

export namespace HierarchyUtils
{
    [[nodiscard]]
    const HierarchyComponent& getHierarchy(const World& world, Entity entity);

    bool hasChildren(const World& world, Entity entity);

    std::generator<Entity> children(const World& world, Entity parent);

    void setParent(World&, Entity child, Entity parent);

    void detach(World&, Entity child);

    Entity getParent(const World&, Entity);

    bool isDescendantOf(const World&, Entity entity, Entity potentialAncestor);
}

template<>
JsonObject serialize(const HierarchyComponent &component, Json::MemoryPoolAllocator<> &allocator)
{
    JsonObject json{Json::kObjectType};

    json.AddMember("parent", JsonObject{PersistentIdUtils::getUuid(component.parent).toString().data(), allocator}, allocator);
    json.AddMember("firstChild", JsonObject{PersistentIdUtils::getUuid(component.firstChild).toString().data(), allocator}, allocator);
    json.AddMember("nextSibling", JsonObject{PersistentIdUtils::getUuid(component.nextSibling).toString().data(), allocator}, allocator);
    json.AddMember("previousSibling", JsonObject{PersistentIdUtils::getUuid(component.previousSibling).toString().data(), allocator}, allocator);

    return json;
}

template<>
HierarchyComponent deserialize(const JsonObject &data)
{
    auto getEntity = [&](const char* linkName)
    {
        const Guid guid = Guid::createFromString(data.FindMember(linkName)->value.GetString());
        return PersistentIdUtils::getEntity(guid);
    };

    return
    {
        .parent = getEntity("parent"),
        .firstChild = getEntity("firstChild"),
        .nextSibling = getEntity("nextSibling"),
        .previousSibling = getEntity("previousSibling"),
    };
}
