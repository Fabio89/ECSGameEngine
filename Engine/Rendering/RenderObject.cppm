export module Render.RenderObject;
import Assets.Mesh;
import Assets.Texture;
import Core;
import ThreadSafeQueue;
import Guid;
import Math;
import Render.Vulkan;

export struct Camera
{
    Mat4 view{};
    Mat4 proj{};
};

struct Texture
{
    std::size_t id{};
    vk::Image image{};
    vk::DeviceMemory memory{};
    vk::ImageView view{};
    vk::Sampler sampler{}; // TODO: should be shared between textures??
};

struct Mesh
{
    std::size_t id{};
    MeshData data;
    vk::Buffer vertexBuffer{};
    vk::DeviceMemory vertexBufferMemory{};
    vk::Buffer indexBuffer{};
    vk::DeviceMemory indexBufferMemory{};
};

struct LineMesh
{
    std::size_t id{};
    std::vector<LineVertex> vertices;
    vk::Buffer vertexBuffer{};
    vk::DeviceMemory vertexBufferMemory{};
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
    bool visible{true};
    std::size_t mesh{};
    std::size_t texture{};
    Mat4 model{1};
    std::vector<vk::Buffer> uniformBuffers;
    std::vector<vk::DeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;
    std::vector<vk::DescriptorSet> descriptorSets;
};

struct LineRenderObject
{
    Entity entity{};
    bool visible{true};
    Mat4 model{1};
    std::vector<LineVertex> vertices;
    vk::Buffer vertexBuffer{};
    vk::DeviceMemory vertexBufferMemory{};
    std::vector<vk::Buffer> uniformBuffers;
    std::vector<vk::DeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;
    std::vector<vk::DescriptorSet> descriptorSets;
};

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
    void setCamera(const Camera& camera);

    void addRenderObject(Entity entity, Guid meshAsset, Guid textureAsset);
    void removeRenderObject(Entity entity);
    void setObjectTransform(Entity entity, const Mat4& worldTransform);
    void addLineRenderObject(Entity entity, std::vector<LineVertex>&& vertices);
    void removeLineRenderObject(Entity entity);
    void setObjectVisibility(Entity entity, bool visible);
    
    void renderFrame
    (
        vk::CommandBuffer commandBuffer,
        vk::PipelineLayout pipelineLayout,
        UInt32 currentFrame
    );

    void renderLineFrame
    (
        vk::CommandBuffer commandBuffer,
        vk::PipelineLayout pipelineLayout,
        UInt32 currentFrame
    );

private:
    void addMeshIfMissing(const Guid& guid);
    void addTextureIfMissing(const Guid& guid);
    void updateDescriptorSets(const RenderObject& object) const;
    void updateUniformBuffer(RenderObject& object, UInt32 currentImage);
    void updateUniformBuffer(LineRenderObject& object, UInt32 currentImage);
    void updateLineDescriptorSets(const LineRenderObject& object) const;
    void removeRenderObject(const RenderObject& object);
    void removeLineRenderObject(const LineRenderObject& object);

    std::vector<RenderObject> m_objects;
    std::vector<LineRenderObject> m_lineObjects;
    std::vector<Mesh> m_meshes;
    std::vector<Texture> m_textures;

    std::unordered_map<Guid, std::size_t> m_meshMap;
    std::unordered_map<Guid, std::size_t> m_textureMap;

    vk::Device m_device{};
    vk::PhysicalDevice m_physicalDevice{};
    vk::SurfaceKHR m_surface{};
    vk::DescriptorPool m_descriptorPool{};
    vk::DescriptorSetLayout m_descriptorSetLayout{};
    vk::Queue m_queue{};
    vk::CommandPool m_cmdPool{};
    Camera m_camera{};
};
