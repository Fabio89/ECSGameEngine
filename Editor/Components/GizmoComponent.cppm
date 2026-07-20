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
    ScaleXY,
    ScaleXZ,
    ScaleYZ,
    ScaleUniform
};

export struct GizmoHandle
{
    Entity entity;
    GizmoHandleType type;
};

export struct GizmoComponent
{
    std::vector<GizmoHandle> handles;
};

export namespace GizmoUtils
{
    template<typename Fn>
    void forEachHandle(const GizmoComponent& gizmo, Fn&& func)
    {
        for (const GizmoHandle& handle : gizmo.handles)
            func(handle.entity);
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
