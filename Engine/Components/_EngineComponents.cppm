export module EngineComponents;
export import Components.BoundingBox;
export import Components.Camera;
export import Components.LineRender;
export import Components.Model;
export import Components.Name;
export import Components.Hierarchy;
export import Components.PersistentId;
export import Components.Render;
export import Components.Tags;
export import Components.Transform;
export import ComponentRegistry;

namespace EngineComponents
{
    export void init()
    {
        ComponentRegistry::init<BoundingBoxComponent>();
        ComponentRegistry::init<CameraComponent>();
        ComponentRegistry::init<LineRenderComponent>();
        ComponentRegistry::init<HierarchyComponent>();
        ComponentRegistry::init<ModelComponent>();
        ComponentRegistry::init<NameComponent>();
        ComponentRegistry::init<PersistentIdComponent>();
        ComponentRegistry::init<RenderComponent>();
        ComponentRegistry::init<RuntimeTransformComponent>();
        ComponentRegistry::init<TagsComponent>();
        ComponentRegistry::init<TransformComponent>();
    }
}