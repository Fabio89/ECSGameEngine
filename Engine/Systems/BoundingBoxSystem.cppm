export module System.BoundingBox;
import Component.BoundingBox;
import Component.Transform;
import Engine.WorldManager;
import Engine.SystemManager;
import Math;
import World.Events;

namespace
{
    EventSubscription subscription;
}

constexpr std::vector<Vec3> computeCorners(const BoundingBoxComponent& aabb)
{
    return
    {
        Vec3{aabb.minLocal.x, aabb.minLocal.y, aabb.minLocal.z},
        Vec3{aabb.maxLocal.x, aabb.minLocal.y, aabb.minLocal.z},
        Vec3{aabb.minLocal.x, aabb.maxLocal.y, aabb.minLocal.z},
        Vec3{aabb.maxLocal.x, aabb.maxLocal.y, aabb.minLocal.z},
        Vec3{aabb.minLocal.x, aabb.minLocal.y, aabb.maxLocal.z},
        Vec3{aabb.maxLocal.x, aabb.minLocal.y, aabb.maxLocal.z},
        Vec3{aabb.minLocal.x, aabb.maxLocal.y, aabb.maxLocal.z},
        Vec3{aabb.maxLocal.x, aabb.maxLocal.y, aabb.maxLocal.z}
    };
}

void computeWorldCorners(BoundingBoxComponent& aabb, const Mat4& worldTransform)
{
    const std::vector<Vec3>& corners = computeCorners(aabb);

    Vec3 minWorld{std::numeric_limits<float>::max()};
    Vec3 maxWorld{std::numeric_limits<float>::lowest()};
    
    for (int i = 0; i < 8; i++)
    {
        const Vec3 worldPos{worldTransform * Vec4{corners[i], 1.0f}};
        minWorld = Math::min(minWorld, worldPos);
        maxWorld = Math::max(maxWorld, worldPos);
    }

    aabb.minWorld = minWorld;
    aabb.maxWorld = maxWorld;
}

void init(SystemContext& context)
{
    subscription += context.worlds.subscribe([&worlds = context.worlds](const WorldEvents::ComponentAdded& event)
    {
        if (event.componentType == getTypeId<BoundingBoxComponent>())
        {
            World& world = worlds.get(event.world);
            auto& aabb = world.editComponent<BoundingBoxComponent>(event.entity);
            const auto& transform = world.readComponent<TransformComponent>(event.entity);
            computeWorldCorners(aabb, transform.runtimeData.worldMatrix);
        }
    });
}

void update(SystemContext& context, float)
{
    context.worlds.forEachWorld([](World& world)
    {
        for (auto&& [entity, aabb, transform] : world.view<BoundingBoxComponent, TransformComponent>())
        {
            computeWorldCorners(aabb, transform.runtimeData.worldMatrix);
        }
    });
}

void shutdown(SystemContext&)
{
    subscription.clear();
}

export namespace BoundingBoxSystem
{
    SystemCallbacks callbacks{.init = init, .update = update, .shutdown = shutdown};
}
