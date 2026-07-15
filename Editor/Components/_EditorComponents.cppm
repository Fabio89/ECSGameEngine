export module Editor.Components;
export import Components.Gizmo;
export import ComponentRegistry;

namespace EditorComponents
{
    export void init()
    {
        ComponentRegistry::init<GizmoComponent>();
    }
}