export module Render.Commands;
import Assets.Mesh;
import Assets.Texture;
import Core;
import Engine.Camera;
import Guid;
import Math;
import Render.RenderLayer;
import WorldHandle;

export namespace RenderCommands
{
    struct AddWorld
    {
        WorldHandle world;
    };

    struct RemoveWorld
    {
        WorldHandle world;
    };

    struct AddObject
    {
        WorldHandle world;
        Entity entity;
        Guid mesh;
        Guid texture;
        Mat4 worldTransform{1};
        RenderLayer layer{RenderLayer::World};
        Vec4 tint{1};
    };

    struct RemoveObject
    {
        WorldHandle world;
        Entity entity;
    };

    struct AddLineObject
    {
        WorldHandle world;
        Entity entity;
        std::vector<LineVertex> vertices;
        Mat4 worldTransform{1};
        bool hidden{};
    };

    struct RemoveLineObject
    {
        WorldHandle world;
        Entity entity;
    };

    struct SetObjectVisibility
    {
        WorldHandle world;
        Entity entity;
        bool visible;
    };

    struct SetTransform
    {
        WorldHandle world;
        Entity entity;
        Mat4 worldTransform;
    };

    struct ClearRenderObjects
    {
        WorldHandle world;
    };

    struct SetCamera
    {
        WorldHandle world;
        Camera camera;
    };
}
