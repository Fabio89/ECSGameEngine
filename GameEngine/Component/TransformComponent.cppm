export module Engine:Component.Transform;
import :Render.RenderManager;
import :Serialisation;
import :System;
import :World;

export struct TransformComponent : Component<TransformComponent>
{
    vec3 position;
    vec3 rotation;
    float scale{1.f};
};

template <>
TransformComponent deserialize(const JsonObject& data)
{
    const vec3 position = Json::parseVec3(data, "position").value_or(vec3{});
    const vec3 rotation = Json::parseVec3(data, "rotation").value_or(vec3{});
    
    float scale{1.f};
    if (const auto it = data.FindMember("scale"); it != data.MemberEnd())
    {
        scale = it->value.GetFloat();
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
