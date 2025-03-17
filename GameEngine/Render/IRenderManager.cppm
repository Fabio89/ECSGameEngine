export module Engine:Render.IRenderManager;
import :Core;
import :IDebugWidget;
import :Render.Model;
import std;

export struct IRenderManager
{
    virtual ~IRenderManager() noexcept = default;
    virtual void init(GLFWwindow* window) = 0;
    virtual void update() = 0;
    virtual void shutdown() = 0;
    virtual bool hasBeenInitialised() const = 0;
    virtual void clear() = 0;
    virtual void addDebugWidget(std::unique_ptr<IDebugWidget> widget) = 0;
    virtual void addRenderObject(Entity entity, const MeshAsset* mesh, const TextureAsset* texture) = 0;
    virtual void setRenderObjectTransform(Entity entity, vec3 location, vec3 rotation, float scale = 1.f) = 0;
};