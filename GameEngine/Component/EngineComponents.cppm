export module Engine:Components;
import :ComponentRegistry;
import :Component.Model;
import :Component.Name;
import :Component.Transform;

namespace EngineComponents
{
    export void init()
    {
        static ComponentRegistry::Entry<ModelComponent> model;
        static ComponentRegistry::Entry<NameComponent> name;
        static ComponentRegistry::Entry<TransformComponent> transform;
    }
}