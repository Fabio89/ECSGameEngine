export module Engine.RenderManager;
import Engine.Decl;
import Engine.DebugWidget;
import Engine.Render.Core;
import std;
import <glm/glm.hpp>;

export struct IRenderManager
{
    virtual ~IRenderManager() noexcept = default;
    virtual void addDebugWidget(std::unique_ptr<DebugWidget> widget) = 0;
    virtual void addRenderObject(Entity entity, const MeshAsset* mesh, const TextureAsset* texture) = 0;
    virtual void setRenderObjectTransform(Entity entity, glm::vec3 location, glm::vec3 rotation, float scale = 1.f) = 0;
};
