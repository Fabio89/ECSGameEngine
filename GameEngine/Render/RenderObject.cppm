export module Render.RenderObject;
import Core;
import CoreTypes;
import Guid;
import Math;
import Render.Model;
import Wrapper.Vulkan;

export struct Camera
{
    Mat4 view{};
    Mat4 proj{};
};

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

struct LineMesh
{
    MeshId id{};
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
    MeshId mesh{invalidId()};
    TextureId texture{invalidId()};
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

    void addMesh(MeshData&& data, Guid guid);
    void addTexture(TextureData&& textureData, Guid guid);
    void addRenderObject(Entity entity, Guid meshAsset, Guid textureAsset);
    void setObjectTransform(Entity entity, Vec3 location = {}, Quat rotation = {}, float scale = 1.f);
    void addLineRenderObject(Entity entity, std::vector<LineVertex>&& vertices);
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
    void updateDescriptorSets(const RenderObject& object) const;
    void updateUniformBuffer(RenderObject& object, UInt32 currentImage);
    void updateUniformBuffer(LineRenderObject& object, UInt32 currentImage);
    void updateLineDescriptorSets(const LineRenderObject& object) const;

    std::vector<RenderObject> m_objects;
    std::vector<LineRenderObject> m_lineObjects;
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
