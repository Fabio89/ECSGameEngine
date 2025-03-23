export module Engine:Components;
import :ComponentRegistry;
import :Component.Camera;
import :Component.Model;
import :Component.Name;
import :Component.Transform;

namespace EngineComponents
{
    export void init()
    {
        ComponentRegistry::init<CameraComponent>();
        ComponentRegistry::init<ModelComponent>();
        ComponentRegistry::init<NameComponent>();
        ComponentRegistry::init<TransformComponent>();
    }
}