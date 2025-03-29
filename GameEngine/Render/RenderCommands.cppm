export module Render.Commands;
import Core;
import Guid;
import Math;
import Render.Model;

export namespace RenderCommands
{
    struct AddMesh
    {
        Guid guid;
        MeshData data;
    };

    struct AddTexture
    {
        Guid guid;
        TextureData data;
    };
    
    struct AddObject
    {
        Entity entity;
        Guid mesh;
        Guid texture;
    };

    struct AddLineObject
    {
        Entity entity;
        std::vector<LineVertex> vertices;
    };

    struct SetObjectVisibility
    {
        Entity entity;
        bool visible;
    };

    struct SetTransform
    {
        Entity entity;
        Vec3 location{};
        Quat rotation{};
        float scale{1.f};
    };
}
