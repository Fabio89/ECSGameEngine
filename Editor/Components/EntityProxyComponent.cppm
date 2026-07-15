export module Components.EntityProxy;
import Core;
import WorldHandle;

export struct EntityProxyComponent
{
    WorldHandle sourceWorld;
    Entity sourceEntity;
};

template<>
constexpr std::string_view getTypeName<EntityProxyComponent>() { return "EntityProxyComponent"; }
