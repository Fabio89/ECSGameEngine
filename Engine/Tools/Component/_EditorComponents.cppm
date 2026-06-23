export module Editor.Components;
export import Component.Gizmo;
export import ComponentRegistry;

namespace EditorComponents
{
    export void init()
    {
        ComponentRegistry::init<GizmoComponent>();
    }
}