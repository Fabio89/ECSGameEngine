export module Editor.Gizmos;
import Core;
import Editor.EntityEditingMode;
import World;

export namespace Gizmos
{
    Entity createTransformGizmo(World& world, EntityEditingMode type);
    Entity createBoundingBoxGizmo(World& editorWorld, const World& sourceEntityWorld,  Entity sourceEntity);
    void setGizmoVisible(World& world, Entity gizmo, bool visible);
}