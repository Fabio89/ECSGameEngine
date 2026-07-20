export module Editor.TransformTool:Detail;
import Editor.EntityEditingMode;
import Editor.TransformTool;
import Math;
import World;

export
{
    GizmoHandleType getHandleType(const World& world, Entity gizmoHandle)
    {
        if (!gizmoHandle.isValid() || !world.hasComponent<GizmoHandleComponent>(gizmoHandle))
            return GizmoHandleType::None;
        return world.readComponent<GizmoHandleComponent>(gizmoHandle).type;
    }

    struct TranslationConstraint
    {
        enum class Type
        {
            Axis,
            Plane
        };

        Type type;
        Vec3 direction;
    };
}

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