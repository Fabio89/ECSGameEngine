module System.Transform;
import Component.Name;
import Component.Hierarchy;
import Component.Transform;
import Math;
import World.Events;

namespace
{
    EventSubscription subscription;

    void updateSubtree(World& world, Entity entity, const Mat4& parentWorld)
    {
        Edit<RuntimeTransformComponent> runtime = world.editComponent<RuntimeTransformComponent>(entity);

        runtime->worldMatrix = parentWorld * TransformUtils::toMatrix(world.readComponent<TransformComponent>(entity));

        for (Entity child : HierarchyUtils::children(world, entity))
        {
            if (world.hasComponent<TransformComponent>(child))
                updateSubtree(world, child, runtime->worldMatrix);
        }
    }

    Mat4 getParentWorldMatrix(const World& world, Entity entity)
    {
        const Entity parent = HierarchyUtils::getParent(world, entity);
        return world.isValid(parent) && world.hasComponent<RuntimeTransformComponent>(parent)
                   ? world.readComponent<RuntimeTransformComponent>(parent).worldMatrix
                   : Mat4{1};
    }
}

void init(SystemContext& context)
{
    subscription += context.worlds.subscribe([&context](const WorldEvents::ComponentAdded& event)
    {
        if (event.componentType == getTypeId<TransformComponent>() || event.componentType == getTypeId<HierarchyComponent>())
        {
            World& world = context.worlds.get(event.world);

            if (!world.hasComponent<RuntimeTransformComponent>(event.entity))
                world.addComponent<RuntimeTransformComponent>(event.entity);

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
            updateSubtree(world, root, getParentWorldMatrix(world, root));
        }
    });
}

void shutdown(SystemContext&)
{
    subscription.clear();
}

void TransformSystem::ensureRuntimeTransform(World& world, Entity entity)
{
    Entity root = entity;

    while (true)
    {
        const Entity parent = HierarchyUtils::getParent(world, root);

        if (!world.isValid(parent))
            break;

        if (!world.isMarked<TransformComponent>(entity)
            && !world.isMarked<HierarchyComponent>(entity))
            break;

        root = parent;
    }

    updateSubtree(world, root, getParentWorldMatrix(world, root));
}
