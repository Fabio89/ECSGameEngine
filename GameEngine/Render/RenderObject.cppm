export module Render.RenderObject;
import CoreTypes;
import Ecs;
import Guid;
import Math;
import Render.IRenderManager;
import Render.Model;
import Wrapper.Vulkan;
import std;

struct Texture
{
    TextureId id{};
    vk::Image image{};
    vk::DeviceMemory memory{};
    vk::ImageView view{};
    vk::Sampler sampler{}; // TODO: should be shared between textures??
};

struct Mesh
{
    MeshId id{};
    MeshData data;
    vk::Buffer vertexBuffer{};
    vk::DeviceMemory vertexBufferMemory{};
    vk::Buffer indexBuffer{};
    vk::DeviceMemory indexBufferMemory{};
};

struct UniformBufferObject
{
    Mat4 model{};
    Mat4 view{};
    Mat4 proj{};
};

struct RenderObject
{
    Entity entity{};
    Mesh mesh;
    Texture texture;
    Vec3 location{};
    Quat rotation{};
    float scale{1.f};
    std::vector<vk::Buffer> uniformBuffers;
    std::vector<vk::DeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;
    std::vector<vk::DescriptorSet> descriptorSets;
};

struct DebugRenderObject
{
    Entity entity{};
    vk::Buffer vertexBuffer{};
    vk::DeviceMemory vertexBufferMemory{};
    std::vector<LineVertex> vertices;
    std::vector<vk::Buffer> uniformBuffers;
    std::vector<vk::DeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;
    std::vector<vk::DescriptorSet> descriptorSets;
};

export namespace RenderMessages
{
    struct AddObject
    {
        Entity entity;
        const MeshAsset* mesh{};
        const TextureAsset* texture{};
    };

    struct SetDebugObject
    {
        Entity entity;
        std::vector<Vec3> vertices;
    };

    struct SetTransform
    {
        Entity entity;
        Vec3 location{};
        Quat rotation{};
        float scale{1.f};
    };
}

export class RenderObjectManager
{
public:
    void init
    (
        vk::Device device,
        vk::PhysicalDevice physicalDevice,
        vk::SurfaceKHR surface,
        vk::DescriptorPool descriptorPool,
        vk::DescriptorSetLayout descriptorSetLayout,
        vk::Queue queue,
        vk::CommandPool cmdPool
    );

    void clear();
    void addCommand(RenderMessages::AddObject command);
    void addCommand(RenderMessages::SetTransform command);
    void addCommand(RenderMessages::SetDebugObject command);
    void executePendingCommands();
    void setCamera(const Camera& camera);

    const Mesh& addMesh(MeshData data, Guid guid);
    const Texture& addTexture(const TextureData& textureData, Guid guid);

    void renderFrame
    (
        vk::CommandBuffer commandBuffer,
        vk::Pipeline graphicsPipeline,
        vk::PipelineLayout pipelineLayout,
        float deltaTime,
        UInt32 currentFrame
    );

    void renderLineFrame
    (
        vk::CommandBuffer commandBuffer,
        vk::Pipeline graphicsPipeline,
        vk::PipelineLayout pipelineLayout,
        float deltaTime,
        UInt32 currentFrame
    );

private:
    void addRenderObject(Entity entity, const MeshAsset& meshAsset, const TextureAsset& textureAsset);
    void setObjectTransform(Entity entity, Vec3 location = {}, Quat rotation = {}, float scale = 1.f);
    void setDebugRenderObject(Entity entity, const std::vector<Vec3>& vertices);
    void updateDescriptorSets(const RenderObject& object) const;
    void updateUniformBuffer(RenderObject& object, UInt32 currentImage, float deltaTime);
    void updateUniformBuffer(DebugRenderObject& object, UInt32 currentImage, float deltaTime);
    void updateLineDescriptorSets(const DebugRenderObject& object) const;

    ThreadSafeQueue<RenderMessages::AddObject> m_addObjectCommands;
    ThreadSafeQueue<RenderMessages::SetTransform> m_setTransformCommands;
    ThreadSafeQueue<RenderMessages::SetDebugObject> m_setDebugObjectCommands;

    std::vector<RenderObject> m_objects;
    std::vector<DebugRenderObject> m_debugObjects;
    std::vector<Mesh> m_meshes;
    std::vector<Texture> m_textures;

    std::unordered_map<Guid, MeshId> m_meshMap;
    std::unordered_map<Guid, TextureId> m_textureMap;

    vk::Device m_device{};
    vk::PhysicalDevice m_physicalDevice{};
    vk::SurfaceKHR m_surface{};
    vk::DescriptorPool m_descriptorPool{};
    vk::DescriptorSetLayout m_descriptorSetLayout{};
    vk::Queue m_queue{};
    vk::CommandPool m_cmdPool{};
    Camera m_camera{};
};
