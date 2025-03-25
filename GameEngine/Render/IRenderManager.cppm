export module Render.IRenderManager;
import Ecs;
import DebugUI.IDebugWidget;
import Math;
import Render.Model;
import Wrapper.Glfw;
import std;

export struct Camera
{
    Mat4 view{};
    Mat4 proj{};
};

export struct IRenderManager
{
    virtual ~IRenderManager() noexcept = default;
    virtual void init(GLFWwindow* window) = 0;
    virtual void update() = 0;
    virtual void shutdown() = 0;
    virtual bool hasBeenInitialized() const = 0;
    virtual void clear() = 0;
    virtual void addDebugWidget(std::unique_ptr<IDebugWidget> widget) = 0;
    virtual void setDebugRenderObject(Entity entity, const std::vector<Vec3>& vertices) = 0;
    virtual void addRenderObject(Entity entity, const MeshAsset* mesh, const TextureAsset* texture) = 0;
    virtual void setRenderObjectTransform(Entity entity, Vec3 location, Quat rotation, float scale = 1.f) = 0;
    virtual void setCamera(const Camera& camera) = 0;
    virtual float getAspectRatio() const = 0;
};
