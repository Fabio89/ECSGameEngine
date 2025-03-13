module Engine:Render.RenderObject;
import :Render.RenderObject;
import :Render.TextureLoading;
import :Render.Vulkan;

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
}

const Texture& RenderObjectManager::addTexture(const TextureData& textureData, Guid guid)
{
    static std::atomic<TextureId> currentId{};

    Texture& texture = m_textures.emplace_back();
    texture.id = currentId++;
    const std::string fullPath = Config::getContentRoot() + textureData.path;
    std::tie(texture.image, texture.memory) = RenderUtils::createTextureImage(
        fullPath.c_str(), m_device, m_physicalDevice,
        m_queue, m_cmdPool);
    texture.view = RenderUtils::createTextureImageView(m_device, texture.image);
    texture.sampler = RenderUtils::createTextureSampler(m_device, m_physicalDevice);
    m_textureMap.emplace(guid, texture.id);
    std::cout << "Added texture: " << guid << std::endl;
    return texture;
}

void RenderObjectManager::updateDescriptorSets(const RenderObject& object) const
{
    const bool hasTexture = object.texture.image;
    const uint32_t descriptorWriteCount = static_cast<uint32_t>(hasTexture) + 1;

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
void RenderObjectManager::updateUniformBuffer(RenderObject& object, vk::Extent2D swapchainExtent, uint32_t currentImage,
                                              float deltaTime)
{
    static float timeElapsed = 0.f;

    //rotate(mat4(1.0f), timeElapsed * radians(90.0f), vec3(0.0f, 0.0f, 1.0f))
    UniformBufferObject uniformBufferObject
    {
        .model = rotate(mat4(1), radians(object.rotation.x), {1, 0, 0})
        * rotate(mat4(1), radians(object.rotation.y), {0, 1, 0})
        * rotate(mat4(1), radians(object.rotation.z), {0, 0, 1})
        * translate(mat4(1), object.location)
        * scale(mat4(1), vec3{object.scale, object.scale, object.scale}),
        .view = lookAt(vec3(2.0f, 2.0f, 2.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f)),
        .proj = perspective(radians(45.0f),
                                 swapchainExtent.width / static_cast<float>(swapchainExtent.height), 0.1f, 10.0f),
    };
    uniformBufferObject.proj[1][1] *= -1;

    memcpy(object.uniformBuffersMapped[currentImage], &uniformBufferObject, sizeof(uniformBufferObject));
    timeElapsed += deltaTime;
}

const Mesh& RenderObjectManager::addMesh(MeshData data, Guid guid)
{
    if (!m_device)
        throw std::runtime_error("RenderObjectManager::addMesh: Device is null");
    
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

void RenderObjectManager::shutdown()
{
    for (const RenderObject& object : m_objects)
    {
        for (auto&& [buffer, memory] : std::views::zip(object.uniformBuffers, object.uniformBuffersMemory))
        {
            m_device.destroyBuffer(buffer);
            m_device.freeMemory(memory);
        }
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
            throw std::runtime_error("Tried to add a render object with invalid mesh!");
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
            throw std::runtime_error("Tried to add a render object with invalid texture!");
        }
    }

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
        .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data(),
    };

    object.descriptorSets = m_device.allocateDescriptorSets(allocInfo);

    m_objects.emplace_back(std::move(object));
    std::cout << "Added render object\n\tMesh: " << meshAsset.getGuid() << "\n\tTexture: " << textureAsset.getGuid() << std::endl;
}

void RenderObjectManager::setObjectTransform(Entity entity, vec3 location, vec3 rotation, float scale)
{
    const auto it = std::ranges::find_if(m_objects, [&](auto&& object) { return object.entity == entity; });
    if (it != m_objects.end())
    {
        it->location = location;
        it->rotation = rotation;
        it->scale = scale;
    }
}

void RenderObjectManager::renderFrame(vk::CommandBuffer commandBuffer, vk::Pipeline graphicsPipeline,
                                      vk::PipelineLayout pipelineLayout, vk::Extent2D swapchainExtent, float deltaTime,
                                      uint32_t currentFrame)
{
    for (RenderObject& object : m_objects)
    {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

        updateDescriptorSets(object);

        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1,
                                         &object.descriptorSets[currentFrame], 0, nullptr);

        const vk::Buffer vertexBuffers[] = {object.mesh.vertexBuffer};
        constexpr vk::DeviceSize offsets[] = {0};

        updateUniformBuffer(object, swapchainExtent, currentFrame, deltaTime);

        commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);
        commandBuffer.bindIndexBuffer(object.mesh.indexBuffer, 0, MeshData::indexType);
        commandBuffer.drawIndexed(static_cast<uint32_t>(object.mesh.data.indices.size()), 1, 0, 0, 0);
    }
}
