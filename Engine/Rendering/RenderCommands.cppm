export module Render.Commands;
import Assets.Mesh;
import Assets.Texture;
import Core;
import Guid;
import Math;

export namespace RenderCommands
{
    struct AddObject
    {
        Entity entity;
        Guid mesh;
        Guid texture;
    };

    struct RemoveObject
    {
        Entity entity;
    };

    struct AddLineObject
    {
        Entity entity;
        std::vector<LineVertex> vertices;
    };

    struct RemoveLineObject
    {
        Entity entity;
    };

    struct SetObjectVisibility
    {
        Entity entity;
        bool visible;
    };

    struct SetTransform
    {
        Entity entity;
        Mat4 worldTransform;
    };

    struct ClearRenderObjects
    {};
}
