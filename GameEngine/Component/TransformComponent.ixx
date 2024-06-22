export module Engine.Component.Transform;
import Engine.Render.Core;
import Engine.Core;
import Engine.World;

export struct TransformComponent : Component<TransformComponent>
{
    TextureId texture;
    MeshId mesh;
};

class ModelSystem : public System
{
public:
//     void addEntity(Entity entity, World& world)
//     {
//         TransformComponent& transform = world.editComponent<TransformComponent>(entity);
//         world.observeOnComponentAdded([this](Entity entity, ComponentTypeId componentType) { onComponentAdded(entity, componentType); });
//     }
//
// private:
//     void onComponentAdded(Entity entity, ComponentTypeId componentType)
//     {
//         if (componentType == TransformComponent::typeId)
//         {
//         
//         }
//     }
};
