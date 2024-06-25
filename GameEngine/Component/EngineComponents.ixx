export module Engine.Components;
import Engine.ComponentRegistry;
import Engine.Component.Model;
import Engine.Component.Transform;

namespace EngineComponents
{
    export void init()
    {
        using ComponentRegistry::registerComponent;
        registerComponent<ModelComponent>();
        registerComponent<TransformComponent>();
    }
}