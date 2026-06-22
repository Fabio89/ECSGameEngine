export module Component.Gizmo;
import Core;

export struct GizmoComponent
{
    Entity xAxisEntity{invalidId()};
    Entity yAxisEntity{invalidId()};
    Entity zAxisEntity{invalidId()};
};

template <>
struct TypeTraits<GizmoComponent>
{
    static constexpr auto name = "GizmoComponent";
};
