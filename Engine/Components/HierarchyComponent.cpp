module Components.Hierarchy;

const HierarchyComponent& HierarchyUtils::getHierarchy(const World& world, Entity entity)
{
    if (!world.isValid(entity) || !world.hasComponent<HierarchyComponent>(entity))
    {
        static constexpr HierarchyComponent noHierarchy{};
        return noHierarchy;
    }
    return world.readComponent<HierarchyComponent>(entity);
}

bool HierarchyUtils::hasChildren(const World& world, Entity entity)
{
    return getHierarchy(world, entity).firstChild.isValid();
}

[[nodiscard]]
std::generator<Entity> HierarchyUtils::children(const World& world, Entity parent)
{
    Entity child = getHierarchy(world, parent).firstChild;

    while (world.isValid(child))
    {
        co_yield child;
        child = getHierarchy(world, child).nextSibling;
    }
}

void HierarchyUtils::setParent(World& world, Entity child, Entity parent)
{
    auto childHierarchy = world.editComponent<HierarchyComponent>(child);
    if (childHierarchy->parent == parent)
        return;

    // Update neighboring siblings
    if (childHierarchy->nextSibling.isValid())
        world.editComponent<HierarchyComponent>(childHierarchy->nextSibling)->previousSibling = childHierarchy->previousSibling;

    if (childHierarchy->previousSibling.isValid())
        world.editComponent<HierarchyComponent>(childHierarchy->previousSibling)->nextSibling = childHierarchy->nextSibling;

    // If it was the first child, let the next sibling take its place
    if (childHierarchy->parent.isValid())
    {
        auto oldParentHierarchy = world.editComponent<HierarchyComponent>(childHierarchy->parent);
        if (oldParentHierarchy->firstChild == child)
        {
            oldParentHierarchy->firstChild = childHierarchy->nextSibling;
        }
    }

    // Reset siblings and link new Parent
    childHierarchy->previousSibling = {};
    childHierarchy->nextSibling = {};
    childHierarchy->parent = parent;

    // Update the new parent
    if (parent.isValid())
    {
        if (!world.hasComponent<HierarchyComponent>(parent))
        {
            world.addComponent<HierarchyComponent>(parent, {.firstChild = child});
        }
        else
        {
            auto newParentHierarchy = world.editComponent<HierarchyComponent>(parent);
            if (!newParentHierarchy->firstChild.isValid())
            {
                newParentHierarchy->firstChild = child;
            }
            else
            {
                Entity lastSibling = newParentHierarchy->firstChild;
                while (lastSibling.isValid())
                {
                    auto siblingHierarchy = world.editComponent<HierarchyComponent>(lastSibling);
                    if (!siblingHierarchy->nextSibling.isValid())
                    {
                        // Find the last of the new siblings and update links.
                        siblingHierarchy->nextSibling = child;
                        childHierarchy->previousSibling = lastSibling;
                        break;
                    }
                    lastSibling = siblingHierarchy->nextSibling;
                }
            }
        }
    }
}

void HierarchyUtils::detach(World& world, Entity child)
{
    setParent(world, child, {});
}

Entity HierarchyUtils::getParent(const World& world, Entity entity)
{
    return getHierarchy(world, entity).parent;
}

bool HierarchyUtils::isDescendantOf(const World& world, Entity entity, Entity potentialAncestor)
{
    for (Entity parent = getParent(world, entity); parent.isValid(); parent = getParent(world, parent))
    {
        if (parent == potentialAncestor)
            return true;
    }
    return false;
}
