export module EngineComponents;
export import Component.BoundingBox;
export import Component.Camera;
export import Component.LineRender;
export import Component.Model;
export import Component.Name;
export import Component.Hierarchy;
export import Component.PersistentId;
export import Component.Render;
export import Component.Tags;
export import Component.Transform;
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
        ComponentRegistry::init<TagsComponent>();
        ComponentRegistry::init<TransformComponent>();
    }
}