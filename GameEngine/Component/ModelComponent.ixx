export module Engine.Component.Model;
import Engine.Render.Application;
import Engine.Render.Core;
import Engine.Core;
import Engine.World;

export struct ModelComponent : Component<ModelComponent>
{
    TextureId texture;
    MeshId mesh;
};

export class ModelSystem : public System
{
public:
    void addEntity(Entity entity, World& world)
    {
        ModelComponent& model = world.editComponent<ModelComponent>(entity);

        renderObjectManager.createRenderObject(model.mesh, model.texture);
    }
};
