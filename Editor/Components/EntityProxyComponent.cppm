export module Components.EntityProxy;
import Components.Hierarchy;
import Components.Transform;
import World;
import WorldHandle;

export enum class EntityProxyFlags : UInt64
{
    None = 0,
    CopyPosition = 1 << 0,
    CopyRotation = 1 << 1,
    CopyScale = 1 << 2,
    CopyTransform = CopyPosition | CopyRotation | CopyScale,
    DestroyWithSource = 1 << 3
};

template<>
struct EnableBitMaskOperators<EntityProxyFlags> : std::true_type {};

export struct EntityProxyComponent
{
    WorldHandle sourceWorld;
    Entity sourceEntity;
    EntityProxyFlags flags = EntityProxyFlags::CopyTransform | EntityProxyFlags::DestroyWithSource;
};

template<>
constexpr std::string_view getTypeName<EntityProxyComponent>() { return "EntityProxyComponent"; }

export namespace EntityProxyUtils
{
    void snapToSourceEntity(World& proxyWorld, const World& sourceWorld, Entity entity, const EntityProxyComponent& proxy)
    {
        if (!proxy.sourceEntity.isValid() || !hasAnyFlag(proxy.flags))
            return;

        if (sourceWorld.hasComponent<TransformComponent>(proxy.sourceEntity))
        {
            TransformUtils::editWorldTransform(proxyWorld, entity, [&](TransformComponent& worldTransform)
            {
                const TransformComponent& sourceWorldTransform = TransformUtils::getWorldTransform(sourceWorld, proxy.sourceEntity);

                if (hasFlag(proxy.flags, EntityProxyFlags::CopyPosition))
                    worldTransform.position = sourceWorldTransform.position;
                if (hasFlag(proxy.flags, EntityProxyFlags::CopyRotation))
                    worldTransform.rotation = sourceWorldTransform.rotation;
                if (hasFlag(proxy.flags, EntityProxyFlags::CopyScale))
                    worldTransform.scale = sourceWorldTransform.scale;
            });

            TransformUtils::forceApplyTransform(proxyWorld, entity);
        }
    }

    void snapToSourceEntity(World& proxyWorld, const World& sourceWorld, Entity entity)
    {
        snapToSourceEntity(proxyWorld, sourceWorld, entity, proxyWorld.readComponent<EntityProxyComponent>(entity));
    }
}
