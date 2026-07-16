export module Editor.TransformTool:Detail;
import Components.Gizmo;
import Editor.EntityEditingMode;
import Editor.TransformTool;
import Math;

export class TranslateTool : public TransformToolImpl<EntityEditingMode::Translate>
{
public:
    using TransformToolImpl::TransformToolImpl;
    void update() override;

private:
    GizmoHandleType m_selectedGizmoHandle{GizmoHandleType::None};
    std::optional<Vec3> m_projectedCursorPositionLastFrame;
    std::unordered_map<Entity, Vec3> m_capturedPositions;
};

export class RotateTool : public TransformToolImpl<EntityEditingMode::Rotate>
{
public:
    using TransformToolImpl::TransformToolImpl;
};

export class ScaleTool : public TransformToolImpl<EntityEditingMode::Scale>
{
public:
    using TransformToolImpl::TransformToolImpl;
};