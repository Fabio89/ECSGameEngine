export module Editor.Gizmos;
import Core;
import World;

export namespace EditorUtils
{
    Entity createTranslationGizmo(World& world);

    Entity createRotationGizmo(World& world);

    Entity createScaleGizmo(World& world);

    void setGizmoVisible(World& world, Entity gizmoEntity, bool visible);

    __declspec(dllexport)
    Entity createBoundingBoxGizmo(World& world, Entity parentEntity);
}