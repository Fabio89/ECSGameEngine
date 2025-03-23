module Render.RenderObject;
import Project;
import Render.TextureLoading;
import Render.Utils;

void RenderObjectManager::init
(
    vk::Device device,
    vk::PhysicalDevice physicalDevice,
    vk::SurfaceKHR surface,
    vk::DescriptorPool descriptorPool,
    vk::DescriptorSetLayout descriptorSetLayout,
    vk::Queue queue,
    vk::CommandPool cmdPool
)
{
    m_device = device;
    m_physicalDevice = physicalDevice;
    m_surface = surface;
    m_descriptorPool = descriptorPool;
    m_descriptorSetLayout = descriptorSetLayout;
    m_queue = queue;
    m_cmdPool = cmdPool;
    m_objects.reserve(100);
    m_meshes.reserve(100);
    m_textures.reserve(100);

    setCameraTransform(m_camera.location, m_camera.rotation);
    setCameraFov(m_camera.fov);
}

const Texture& RenderObjectManager::addTexture(const TextureData& textureData, Guid guid)
{
    static std::atomic<TextureId> currentId{};

    Texture& texture = m_textures.emplace_back();
    texture.id = currentId++;
    const std::string fullPath = Project::getContentRoot() + textureData.path;
    std::tie(texture.image, texture.memory) = RenderUtils::createTextureImage(
        fullPath.c_str(), m_device, m_physicalDevice,
        m_queue, m_cmdPool);

    if (texture.image)
    {
        texture.view = RenderUtils::createTextureImageView(m_device, texture.image);
        texture.sampler = RenderUtils::createTextureSampler(m_device, m_physicalDevice);
        m_textureMap.emplace(guid, texture.id);
        std::cout << "Added texture: " << guid << std::endl;
    }

    return texture;
}

void RenderObjectManager::updateDescriptorSets(const RenderObject& object) const
{
    const bool hasTexture = object.texture.image;
    const UInt32 descriptorWriteCount = static_cast<UInt32>(hasTexture) + 1;

    for (size_t i = 0; i < MaxFramesInFlight; i++)
    {
        std::vector<vk::WriteDescriptorSet> descriptorWrites;
        descriptorWrites.reserve(descriptorWriteCount);

        const vk::DescriptorBufferInfo vertexBufferInfo
        {
            .buffer = object.uniformBuffers[i],
            .offset = 0,
            .range = sizeof(UniformBufferObject),
        };
        descriptorWrites.emplace_back
        (
            vk::WriteDescriptorSet
            {
                .dstSet = object.descriptorSets[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eUniformBuffer,
                .pBufferInfo = &vertexBufferInfo,
            }
        );

        if (hasTexture)
        {
            const vk::DescriptorImageInfo imageInfo
            {
                .sampler = object.texture.sampler,
                .imageView = object.texture.view,
                .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
            };
            descriptorWrites.emplace_back
            (
                vk::WriteDescriptorSet
                {
                    .dstSet = object.descriptorSets[i],
                    .dstBinding = 1,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                    .pImageInfo = &imageInfo,
                }
            );
        }

        m_device.updateDescriptorSets
        (
            descriptorWriteCount,
            descriptorWrites.data(),
            0,
            nullptr
        );
    }
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void RenderObjectManager::updateUniformBuffer(RenderObject& object, UInt32 currentImage, float deltaTime)
{
    const Mat4 translationMatrix = Math::translate(Mat4{1.0f}, object.location);
    const Mat4 rotationMatrix = Math::mat4_cast(object.rotation);
    const Mat4 scaleMatrix = Math::scale(Mat4{1.0f}, Vec3{object.scale, object.scale, object.scale});

    UniformBufferObject uniformBufferObject
    {
        .model = translationMatrix * rotationMatrix * scaleMatrix,
        .view = m_view,
        .proj = m_proj,
    };

    std::memcpy(object.uniformBuffersMapped[currentImage], &uniformBufferObject, sizeof(uniformBufferObject));
}

const Mesh& RenderObjectManager::addMesh(MeshData data, Guid guid)
{
    check(m_device, "[RenderObjectManager::addMesh] Device is null");
    if (!check(data.vertices.size() > 0, "[RenderObjectManager::addMesh] Tried to add mesh with no vertices", ErrorType::Warning))
    {
        static const Mesh emptyMesh;
        return emptyMesh;
    }

    Mesh& mesh = m_meshes.emplace_back();
    const RenderUtils::CreateDataBufferInfo vertexBufferInfo
    {
        .device = m_device,
        .physicalDevice = m_physicalDevice,
        .surface = m_surface,
        .usageType = vk::BufferUsageFlagBits::eVertexBuffer,
        .transferQueue = m_queue,
        .transferCommandPool = m_cmdPool,
    };
    std::tie(mesh.vertexBuffer, mesh.vertexBufferMemory) = createDataBuffer(data.vertices, vertexBufferInfo);

    const RenderUtils::CreateDataBufferInfo indexBufferInfo
    {
        .device = m_device,
        .physicalDevice = m_physicalDevice,
        .surface = m_surface,
        .usageType = vk::BufferUsageFlagBits::eIndexBuffer,
        .transferQueue = m_queue,
        .transferCommandPool = m_cmdPool,
    };
    std::tie(mesh.indexBuffer, mesh.indexBufferMemory) = createDataBuffer(data.indices, indexBufferInfo);

    mesh.data = std::move(data);

    static std::atomic<MeshId> currentId{};
    const MeshId id = mesh.id = currentId++;
    m_meshMap.emplace(guid, id);
    std::cout << "Added mesh: " << guid << std::endl;
    return mesh;
}

void RenderObjectManager::clear()
{
    m_addObjectCommands.clear();
    m_setTransformCommands.clear();
    m_meshMap.clear();
    m_textureMap.clear();

    for (const RenderObject& object : m_objects)
    {
        for (auto&& [buffer, memory] : std::views::zip(object.uniformBuffers, object.uniformBuffersMemory))
        {
            m_device.destroyBuffer(buffer);
            m_device.freeMemory(memory);
        }

        check(std::in_range<UInt32>(object.descriptorSets.size()), "Truncating value of object.descriptorSets.size() when freeing descriptor sets!");
        auto result = m_device.freeDescriptorSets(m_descriptorPool, static_cast<UInt32>(object.descriptorSets.size()), object.descriptorSets.data());
        check(result == vk::Result::eSuccess, "[RenderObjectManager::clear] Failed to free descriptor sets!");
    }
    m_objects.clear();

    for (const Mesh& mesh : m_meshes)
    {
        m_device.destroyBuffer(mesh.vertexBuffer);
        m_device.freeMemory(mesh.vertexBufferMemory);
        m_device.destroyBuffer(mesh.indexBuffer);
        m_device.freeMemory(mesh.indexBufferMemory);
    }
    m_meshes.clear();

    for (const Texture& texture : m_textures)
    {
        m_device.destroySampler(texture.sampler);
        m_device.destroyImageView(texture.view);
        m_device.destroyImage(texture.image);
        m_device.freeMemory(texture.memory);
    }
    m_textures.clear();
}

void RenderObjectManager::addCommand(RenderMessages::AddObject command)
{
    m_addObjectCommands.push(std::move(command));
}

void RenderObjectManager::addCommand(RenderMessages::SetTransform command)
{
    m_setTransformCommands.push(std::move(command));
}

void RenderObjectManager::executePendingCommands()
{
    {
        auto cmd = m_addObjectCommands.tryPop();
        while (cmd.has_value())
        {
            addRenderObject(cmd->entity, *cmd->mesh, *cmd->texture);
            cmd = m_addObjectCommands.tryPop();
        }
    }

    {
        auto cmd = m_setTransformCommands.tryPop();
        while (cmd.has_value())
        {
            setObjectTransform(cmd->entity, cmd->location, cmd->rotation, cmd->scale);
            cmd = m_setTransformCommands.tryPop();
        }
    }
}

void RenderObjectManager::addRenderObject(Entity entity, const MeshAsset& meshAsset, const TextureAsset& textureAsset)
{
    RenderObject object;
    static constexpr vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

    const Mesh* mesh;
    {
        if (const auto idIt = m_meshMap.find(meshAsset.getGuid()); idIt == m_meshMap.end())
        {
            mesh = &addMesh(meshAsset.getData(), meshAsset.getGuid());
        }
        else if (const auto it = std::ranges::find_if(m_meshes, [id = idIt->second](auto&& element) { return element.id == id; }); it != m_meshes.end())
        {
            mesh = &*it;
        }
        else
        {
            fatalError("Tried to add a render object with invalid mesh!");
            return;
        }
    }

    const Texture* texture;
    {
        if (const auto idIt = m_textureMap.find(textureAsset.getGuid()); idIt == m_textureMap.end())
        {
            texture = &addTexture(textureAsset.getData(), textureAsset.getGuid());
        }
        else if (const auto it = std::ranges::find_if(m_textures, [id = idIt->second](auto&& element) { return element.id == id; }); it != m_textures.end())
        {
            texture = &*it;
        }
        else
        {
            fatalError("Tried to add a render object with invalid texture!");
            return;
        }
    }

    if (!check(mesh, "Tried to add a render object with null mesh!", ErrorType::Error)
        || !check(!mesh->data.vertices.empty(), "Tried to add a render object with empty mesh!", ErrorType::Warning)
        || !check(texture, "Tried to add a render object with null texture!", ErrorType::Error)
        || !check(texture->image, "Tried to add a render object with empty texture!", ErrorType::Warning))
        return;

    object.entity = entity;
    object.mesh = *mesh;
    object.texture = *texture;
    object.uniformBuffers = std::vector<vk::Buffer>(MaxFramesInFlight);
    object.uniformBuffersMemory = std::vector<vk::DeviceMemory>(MaxFramesInFlight);
    object.uniformBuffersMapped = std::vector<void*>(MaxFramesInFlight);

    const RenderUtils::CreateBufferInfo bufferInfo
    {
        .device = m_device,
        .physicalDevice = m_physicalDevice,
        .size = bufferSize,
        .usage = vk::BufferUsageFlagBits::eUniformBuffer,
        .properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
    };

    auto bufferRange = std::views::zip(object.uniformBuffers, object.uniformBuffersMemory, object.uniformBuffersMapped);

    for (auto&& [buffer, memory, mapped] : bufferRange)
    {
        std::tie(buffer, memory) = createBuffer(bufferInfo);
        if (m_device.mapMemory(memory, 0, bufferSize, {}, &mapped) != vk::Result::eSuccess)
            return;
    }

    std::vector layouts(MaxFramesInFlight, m_descriptorSetLayout);
    const vk::DescriptorSetAllocateInfo allocInfo
    {
        .descriptorPool = m_descriptorPool,
        .descriptorSetCount = static_cast<UInt32>(layouts.size()),
        .pSetLayouts = layouts.data(),
    };

    object.descriptorSets = m_device.allocateDescriptorSets(allocInfo);

    m_objects.emplace_back(std::move(object));
    std::cout << "Added render object\n\tMesh: " << meshAsset.getGuid() << "\n\tTexture: " << textureAsset.getGuid() << std::endl;
}

void RenderObjectManager::setObjectTransform(Entity entity, Vec3 location, Quat rotation, float scale)
{
    const auto it = std::ranges::find_if(m_objects, [&](auto&& object) { return object.entity == entity; });
    if (it != m_objects.end())
    {
        it->location = location;
        it->rotation = rotation;
        it->scale = scale;
    }
}

void RenderObjectManager::setCameraTransform(Vec3 location, Quat rotation)
{
    const Vec3 forward = Math::normalize(forwardVector() * rotation);
    m_camera.location = location;
    m_camera.rotation = rotation;
    m_view = Math::lookAt(location, location + forward, upVector());
}

void RenderObjectManager::setCameraFov(float fov)
{
    m_camera.fov = fov;
}

void RenderObjectManager::setAspectRatio(float aspectRatio)
{
    m_aspectRatio = aspectRatio;
}

void RenderObjectManager::renderFrame
(
    vk::CommandBuffer commandBuffer,
    vk::Pipeline graphicsPipeline,
    vk::PipelineLayout pipelineLayout,
    float deltaTime,
    UInt32 currentFrame
)
{
    m_proj = Math::perspective(Math::radians(m_camera.fov), m_aspectRatio, m_camera.nearPlane, m_camera.farPlane);
    m_proj[1][1] *= -1;
    
    for (RenderObject& object : m_objects)
    {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

        updateDescriptorSets(object);

        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1,
                                         &object.descriptorSets[currentFrame], 0, nullptr);

        const vk::Buffer vertexBuffers[] = {object.mesh.vertexBuffer};
        constexpr vk::DeviceSize offsets[] = {0};

        updateUniformBuffer(object, currentFrame, deltaTime);

        commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);
        commandBuffer.bindIndexBuffer(object.mesh.indexBuffer, 0, MeshData::indexType);
        commandBuffer.drawIndexed(static_cast<UInt32>(object.mesh.data.indices.size()), 1, 0, 0, 0);
    }
}
