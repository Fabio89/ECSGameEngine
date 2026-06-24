export module Component.Gizmo;
import Core;

export struct GizmoComponent
{
    Entity xAxisEntity{invalidId()};
    Entity yAxisEntity{invalidId()};
    Entity zAxisEntity{invalidId()};
};

template<>
constexpr std::string_view getTypeName<GizmoComponent>() { return "GizmoComponent"; }
