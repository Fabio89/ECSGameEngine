module Components.Transform;

Vec3 TransformUtils::forward(const TransformComponent& transform)
{
    return Math::normalize(Math::rotate(transform.rotation, forwardVector()));
}

Vec3 TransformUtils::right(const TransformComponent& transform)
{
    return Math::normalize(Math::rotate(transform.rotation, rightVector()));
}

Vec3 TransformUtils::up(const TransformComponent& transform)
{
    return Math::normalize(Math::rotate(transform.rotation, upVector()));
}

Mat4 TransformUtils::toMatrix(const TransformComponent& transform)
{
    return Math::translate(Mat4{1.0f}, transform.position) * Math::mat4_cast(transform.rotation) * Math::scale(Mat4{1.0f}, transform.scale);
}

TransformComponent TransformUtils::getWorldTransform(const World& world, Entity entity)
{
    const TransformComponent& local = world.readComponent<TransformComponent>(entity);

    const Entity parent = HierarchyUtils::getParent(world, entity);

    if (!world.isValid(parent))
        return local;

    const TransformComponent parentWorld = getWorldTransform(world, parent);

    return {
        .position = parentWorld.position + parentWorld.rotation * (parentWorld.scale * local.position),
        .rotation = parentWorld.rotation * local.rotation,
        .scale = parentWorld.scale * local.scale
    };
}

void TransformUtils::setWorldTransform(World& world, Entity entity, const TransformComponent& worldTransform)
{
    const Entity parent = HierarchyUtils::getParent(world, entity);

    TransformComponent local = worldTransform;

    if (world.isValid(parent))
    {
        const TransformComponent parentWorld = getWorldTransform(world, parent);

        local.scale = worldTransform.scale / parentWorld.scale;
        local.rotation = Math::inverse(parentWorld.rotation) * worldTransform.rotation;
        local.position = Math::inverse(parentWorld.rotation) * ((worldTransform.position - parentWorld.position) / parentWorld.scale);
    }

    auto transform = world.editComponent<TransformComponent>(entity);
    *transform = local;
}

void TransformUtils::editWorldTransform(World& world, Entity entity, const std::function<void(TransformComponent& worldTransform)>& editFunc)
{
    TransformComponent t = getWorldTransform(world, entity);
    editFunc(t);
    setWorldTransform(world, entity, t);
}

namespace
{
    Mat4 getParentWorldMatrix(const World& world, Entity entity)
    {
        const Entity parent = HierarchyUtils::getParent(world, entity);
        return world.isValid(parent) && world.hasComponent<RuntimeTransformComponent>(parent)
                   ? world.readComponent<RuntimeTransformComponent>(parent).worldMatrix
                   : Mat4{1};
    }

    void forceApplyTransform(World& world, Entity entity, const Mat4& parentWorld)
    {
        Edit<RuntimeTransformComponent> runtime = world.editComponent<RuntimeTransformComponent>(entity);

        runtime->worldMatrix = parentWorld * TransformUtils::toMatrix(world.readComponent<TransformComponent>(entity));

        for (Entity child : HierarchyUtils::children(world, entity))
        {
            if (world.hasComponent<TransformComponent>(child))
                forceApplyTransform(world, child, runtime->worldMatrix);
        }
    }
}

void TransformUtils::forceApplyTransform(World& world, Entity entity)
{
    ::forceApplyTransform(world, entity, getParentWorldMatrix(world, entity));
}
