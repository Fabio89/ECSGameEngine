module Systems.Transform;
import Components.Name;
import Components.Hierarchy;
import Components.Transform;
import Math;
import World.Events;

namespace
{
    EventSubscription subscription;
}

void init(SystemContext& context)
{
    context.worlds.forEachWorld([](World& world)
    {
        for (auto&& [entity, transform] : world.query<TransformComponent>())
        {
            TransformSystem::ensureRuntimeTransform(world, entity);
        }
    });

    subscription += context.worlds.subscribe([&context](const WorldEvents::ComponentAdded& event)
    {
        if (event.componentType == getTypeId<TransformComponent>() || event.componentType == getTypeId<HierarchyComponent>())
        {
            World& world = context.worlds.get(event.world);
            if (world.hasComponent<TransformComponent>(event.entity))
                TransformSystem::ensureRuntimeTransform(world, event.entity);
        }
    });
}

void update(SystemContext& context, float)
{
    context.worlds.forEachWorld([](World& world)
    {
        std::vector<Entity> roots;

        auto addRoot = [&](Entity entity)
        {
            if (world.hasComponent<TransformComponent>(entity))
                roots.push_back(entity);
        };

        for (const Entity e : world.getMarked<TransformComponent>())
            addRoot(e);

        for (const Entity e : world.getMarked<HierarchyComponent>())
            addRoot(e);

        // Remove descendants.
        std::erase_if(roots, [&](Entity entity)
        {
            Entity parent = HierarchyUtils::getParent(world, entity);

            while (world.isValid(parent))
            {
                if (std::ranges::contains(roots, parent))
                    return true;

                parent = HierarchyUtils::getParent(world, parent);
            }

            return false;
        });

        for (const Entity root : roots)
        {
            TransformUtils::forceApplyTransform(world, root);
        }
    });
}

void shutdown(SystemContext&)
{
    subscription.clear();
}

void TransformSystem::ensureRuntimeTransform(World& world, Entity entity)
{
    if (!world.hasComponent<RuntimeTransformComponent>(entity))
        world.addComponent<RuntimeTransformComponent>(entity);

    Entity root = entity;

    while (true)
    {
        const Entity parent = HierarchyUtils::getParent(world, root);

        if (!world.isValid(parent))
            break;

        if (!world.isMarked<TransformComponent>(root)
            && !world.isMarked<HierarchyComponent>(root))
            break;

        root = parent;
    }

    TransformUtils::forceApplyTransform(world, root);
}
