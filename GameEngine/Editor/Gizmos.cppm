export module Editor.Gizmos;
import Core;
import World;

export namespace EditorUtils
{
    Entity createTranslationGizmo(World& world);

    Entity createRotationGizmo(World& world);

    Entity createScaleGizmo(World& world);

    __declspec(dllexport)
    Entity createBoundingBoxGizmo(World& world, Entity parentEntity);
}