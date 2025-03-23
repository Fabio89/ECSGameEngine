export module Engine:Render.IRenderManager;
import :Ecs;
import :IDebugWidget;
import :Render.Model;
import Wrapper.Glfw;
import std;

export struct IRenderManager
{
    virtual ~IRenderManager() noexcept = default;
    virtual void init(GLFWwindow* window) = 0;
    virtual void update() = 0;
    virtual void shutdown() = 0;
    virtual bool hasBeenInitialized() const = 0;
    virtual void clear() = 0;
    virtual void addDebugWidget(std::unique_ptr<IDebugWidget> widget) = 0;
    virtual void addRenderObject(Entity entity, const MeshAsset* mesh, const TextureAsset* texture) = 0;
    virtual void setRenderObjectTransform(Entity entity, Vec3 location, Quat rotation, float scale = 1.f) = 0;\
    virtual void setCameraTransform(Vec3 location, Quat rotation) = 0;
    virtual void setCameraFov(float fov) = 0;
};
