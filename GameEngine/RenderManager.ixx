export module Engine.RenderManager;
import Engine.Decl;
import Engine.DebugWidget;
import Engine.Render.Core;
import Math;
import std;

export struct IRenderManager
{
    virtual ~IRenderManager() noexcept = default;
    virtual void addDebugWidget(std::unique_ptr<DebugWidget> widget) = 0;
    virtual void addRenderObject(Entity entity, const MeshAsset* mesh, const TextureAsset* texture) = 0;
    virtual void setRenderObjectTransform(Entity entity, vec3 location, vec3 rotation, float scale = 1.f) = 0;
};
