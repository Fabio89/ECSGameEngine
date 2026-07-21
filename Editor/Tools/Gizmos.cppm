export module Editor.Gizmos;
import AssetManager;
import Core;
import Editor.EntityEditingMode;
import World;

export namespace Gizmos
{
    void init(AssetManager& assets, AssetMountId mount);
    Entity createTransformGizmo(World& editorWorld, WorldHandle mainWorld, EntityEditingMode type);
    Entity createBoundingBoxGizmo(World& editorWorld, const World& sourceEntityWorld,  Entity sourceEntity);
    void setGizmoVisible(World& world, Entity gizmo, bool visible);
}
