export module Component.Gizmo;
import Core;

export struct GizmoComponent
{
    Entity xAxisEntity{};
    Entity yAxisEntity{};
    Entity zAxisEntity{};
};

template<>
constexpr std::string_view getTypeName<GizmoComponent>() { return "GizmoComponent"; }
