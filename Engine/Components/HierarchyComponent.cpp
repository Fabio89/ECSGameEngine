module Component.Hierarchy;

const HierarchyComponent& HierarchyUtils::getHierarchy(const World& world, Entity entity)
{
    if (!world.hasComponent<HierarchyComponent>(entity))
    {
        static constexpr HierarchyComponent noHierarchy{};
        return noHierarchy;
    }
    return world.readComponent<HierarchyComponent>(entity);
}

bool HierarchyUtils::hasChildren(const World& world, Entity entity)
{
    return getHierarchy(world, entity).firstChild != invalidId();
}

[[nodiscard]]
std::generator<Entity> HierarchyUtils::children(const World& world, Entity parent)
{
    Entity child = getHierarchy(world, parent).firstChild;

    while (child != invalidId())
    {
        co_yield child;
        child = getHierarchy(world, child).nextSibling;
    }
}

void HierarchyUtils::setParent(World& world, Entity child, Entity parent)
{
    HierarchyComponent& childHierarchy = world.editComponent<HierarchyComponent>(child);
    if (childHierarchy.parent == parent)
        return;

    // Update neighbouring siblings
    if (childHierarchy.nextSibling != invalidId())
        world.editComponent<HierarchyComponent>(childHierarchy.nextSibling).previousSibling = childHierarchy.previousSibling;

    if (childHierarchy.previousSibling != invalidId())
        world.editComponent<HierarchyComponent>(childHierarchy.previousSibling).nextSibling = childHierarchy.nextSibling;

    // If it was the first child, let the next sibling take its place
    if (childHierarchy.parent != invalidId())
    {
        HierarchyComponent& oldParentHierarchy = world.editComponent<HierarchyComponent>(childHierarchy.parent);
        if (oldParentHierarchy.firstChild == child)
        {
            oldParentHierarchy.firstChild = childHierarchy.nextSibling;
        }
    }

    // Reset siblings and link new Parent
    childHierarchy.previousSibling = invalidId();
    childHierarchy.nextSibling = invalidId();
    childHierarchy.parent = parent;

    // Update the new parent
    if (parent != invalidId())
    {
        if (!world.hasComponent<HierarchyComponent>(parent))
        {
            world.addComponent<HierarchyComponent>(parent, {.firstChild = child});
        }
        else
        {
            HierarchyComponent& newParentHierarchy = world.editComponent<HierarchyComponent>(parent);
            if (newParentHierarchy.firstChild == invalidId())
            {
                newParentHierarchy.firstChild = child;
            }
            else
            {
                Entity lastSibling = newParentHierarchy.firstChild;
                while (lastSibling != invalidId())
                {
                    HierarchyComponent& siblingHierarchy = world.editComponent<HierarchyComponent>(lastSibling);
                    if (siblingHierarchy.nextSibling == invalidId())
                    {
                        // Find the last of the new siblings and update links.
                        siblingHierarchy.nextSibling = child;
                        childHierarchy.previousSibling = lastSibling;
                        break;
                    }
                    lastSibling = siblingHierarchy.nextSibling;
                }
            }
        }
    }
}

void HierarchyUtils::detach(World& world, Entity child)
{
    setParent(world, child, invalidId());
}

Entity HierarchyUtils::getParent(const World& world, Entity entity)
{
    return getHierarchy(world, entity).parent;
}

bool HierarchyUtils::isDescendantOf(const World& world, Entity entity, Entity potentialAncestor)
{
    for (Entity parent = getParent(world, entity); parent != invalidId(); parent = getParent(world, parent))
    {
        if (parent == potentialAncestor)
            return true;
    }
    return false;
}
