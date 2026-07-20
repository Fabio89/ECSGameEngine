export module Editor.TransformTool:Detail;
import Editor.EntityEditingMode;
import Editor.TransformTool;
import Math;

export class TranslateTool : public TransformToolImpl<EntityEditingMode::Translate>
{
public:
    using TransformToolImpl::TransformToolImpl;
    void update() override;

private:
    std::optional<Vec3> m_projectedCursorPositionLastFrame;
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
    void update() override;

private:
    std::optional<Vec3> m_projectedCursorPositionLastFrame;
    std::optional<Vec2> m_previousMousePosition{};
};