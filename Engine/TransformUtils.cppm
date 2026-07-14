export module Engine.TransformUtils;
import Component.Hierarchy;
import Component.Transform;
import Math;
import World;

export Mat4 getParentWorldMatrix(const World& world, Entity entity)
{
    const Entity parent = HierarchyUtils::getParent(world, entity);
    return world.isValid(parent) && world.hasComponent<RuntimeTransformComponent>(parent)
               ? world.readComponent<RuntimeTransformComponent>(parent).worldMatrix
               : Mat4{1};
}

export void updateSubtreeTransforms(World& world, Entity entity, const Mat4& parentWorld)
{
    if (!world.hasComponent<RuntimeTransformComponent>(entity))
        world.addComponent<RuntimeTransformComponent>(entity);

    Edit<RuntimeTransformComponent> runtime = world.editComponent<RuntimeTransformComponent>(entity);

    runtime->worldMatrix = parentWorld * TransformUtils::toMatrix(world.readComponent<TransformComponent>(entity));

    for (Entity child : HierarchyUtils::children(world, entity))
    {
        if (world.hasComponent<TransformComponent>(child))
            updateSubtreeTransforms(world, child, runtime->worldMatrix);
    }
}