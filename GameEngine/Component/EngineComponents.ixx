export module Engine.Components;
import Engine.ComponentRegistry;
import Engine.Component.Model;
import Engine.Component.Name;
import Engine.Component.Transform;

namespace EngineComponents
{
    export void init()
    {
        static ComponentRegistry::Entry<ModelComponent> model;
        static ComponentRegistry::Entry<NameComponent> name;
        static ComponentRegistry::Entry<TransformComponent> transform;
    }
}