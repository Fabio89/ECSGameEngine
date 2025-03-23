export module Engine:Component.Transform;
import :Render.RenderManager;
import :System;
import :World;
import Math;
import Serialization;

export struct TransformComponent : Component<TransformComponent>
{
    Vec3 position;
    Quat rotation;
    float scale{1.f};
};

template <>
struct TypeTraits<TransformComponent>
{
    static constexpr auto name = "TransformComponent";
};

template<>
JsonObject serialize(const TransformComponent& component, Json::MemoryPoolAllocator<>& allocator)
{
    JsonObject json{Json::kObjectType};
    json.AddMember("position", Json::fromVec3(component.position, allocator), allocator);
    json.AddMember("rotation", Json::fromQuat(component.rotation, allocator), allocator);
    json.AddMember("scale", component.scale, allocator);
    return json;
}

template <>
TransformComponent deserialize(const JsonObject& data)
{
    const Vec3 position = Json::toVec3(data, "position").value_or(Vec3{});
    const Quat rotation = Json::toQuat(data, "rotation").value_or(Quat{});
    
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
