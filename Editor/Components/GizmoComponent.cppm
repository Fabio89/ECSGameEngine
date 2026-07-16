export module Components.Gizmo;
import Core;

export enum class GizmoHandleType
{
    None,

    TranslateX,
    TranslateY,
    TranslateZ,

    TranslateXY,
    TranslateXZ,
    TranslateYZ,

    RotateX,
    RotateY,
    RotateZ,

    ScaleX,
    ScaleY,
    ScaleZ,

    ScaleUniform
};

export struct GizmoComponent
{
    Entity xAxis;
    Entity yAxis;
    Entity zAxis;

    Entity xyPlane;
    Entity xzPlane;
    Entity yzPlane;
};

export namespace GizmoUtils
{
    template<typename Fn>
    void forEachHandle(const GizmoComponent& gizmo, Fn&& func)
    {
        func(gizmo.xAxis);
        func(gizmo.yAxis);
        func(gizmo.zAxis);
        func(gizmo.xyPlane);
        func(gizmo.xzPlane);
        func(gizmo.yzPlane);
    }
}

template<>
constexpr std::string_view getTypeName<GizmoComponent>() { return "GizmoComponent"; }

export struct GizmoHandleComponent
{
    GizmoHandleType type{};
};

template<>
constexpr std::string_view getTypeName<GizmoHandleComponent>() { return "GizmoHandleComponent"; }
