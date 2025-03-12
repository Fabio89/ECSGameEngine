export module Engine.Component.Transform;
import Engine.Render.Application;
import Engine.Render.Core;
import Engine.ComponentRegistry;
import Engine.Config;
import Engine.Core;
import Engine.World;
import Math;

export struct TransformComponent : Component<TransformComponent>
{
    vec3 position;
    vec3 rotation;
    float scale{1.f};
};

template <>
TransformComponent deserialize(const Json& data)
{
    vec3 position{};
    if (auto it = data.find("position"); it != data.end())
    {
        position = *it;
    }

    vec3 rotation{};
    if (auto it = data.find("rotation"); it != data.end())
    {
        rotation = *it;
    }

    float scale{1.f};
    if (auto it = data.find("scale"); it != data.end())
    {
        scale = *it;
    }

    return
    {
        .position = position,
        .rotation = rotation,
        .scale = scale
    };
}

export class TransformSystem : public System
{
public:
    void onComponentAdded(World& world, Entity entity, ComponentTypeId componentType) override
    {
        if (componentType == TransformComponent::typeId)
        {
            updateRenderTransform(world, entity);

            addUpdateFunction([&world, entity](float)
            {
                updateRenderTransform(world, entity);
            });
        }
    }

private:
    static void updateRenderTransform(World& world, Entity entity)
    {
        const auto& component = world.readComponent<TransformComponent>(entity);
        world.getRenderManager().setRenderObjectTransform(entity, component.position, component.rotation, component.scale);
    }
};
