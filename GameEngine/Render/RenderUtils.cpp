module Render.Utils;
import Log;
import Render.QueueFamily;

[[nodiscard]] UInt32 RenderUtils::findMemoryType(vk::PhysicalDevice physicalDevice, UInt32 typeFilter,
                                                   vk::MemoryPropertyFlags properties)
{
    vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

    for (UInt32 i = 0; i < memProperties.memoryTypeCount; ++i)
    {
        if ((typeFilter & (1 << i))
            && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    fatalError("failed to find suitable memory type!");
    return 0;
}

vk::CommandBuffer RenderUtils::beginSingleTimeCommands(vk::Device device, vk::CommandPool commandPool)
{
    const vk::CommandBufferAllocateInfo allocInfo
    {
        .commandPool = commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1,
    };

    const vk::CommandBuffer commandBuffer = *device.allocateCommandBuffers(allocInfo).begin();

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    commandBuffer.begin(beginInfo);
    return commandBuffer;
}

void RenderUtils::endSingleTimeCommands(vk::Device device, vk::CommandBuffer buffer, vk::Queue queue,
                                        vk::CommandPool pool)
{
    buffer.end();
    const vk::SubmitInfo submitInfo
    {
        . commandBufferCount = 1,
        . pCommandBuffers = &buffer,
    };

    check(queue.submit(1, &submitInfo, nullptr) == vk::Result::eSuccess, "Failed to submit queue!");

    queue.waitIdle();
    device.freeCommandBuffers(pool, 1, &buffer);
}

std::tuple<vk::Buffer, vk::DeviceMemory> RenderUtils::createBuffer(const CreateBufferInfo& info)
{
    std::tuple<vk::Buffer, vk::DeviceMemory> result;
    auto& [buffer, memory] = result;

    const vk::BufferCreateInfo bufferInfo
    {
        .size = info.size,
        .usage = info.usage,
        .sharingMode = info.queueFamilyIndices.size() > 1u
                           ? vk::SharingMode::eConcurrent
                           : vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = static_cast<UInt32>(info.queueFamilyIndices.size()),
        .pQueueFamilyIndices = info.queueFamilyIndices.begin()
    };

    buffer = info.device.createBuffer(bufferInfo, nullptr);
    if (!buffer)
    {
        fatalError("failed to create vertex buffer!");
    }

    const vk::MemoryRequirements memRequirements = info.device.getBufferMemoryRequirements(buffer);

    const vk::MemoryAllocateInfo allocInfo
    {
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryType(info.physicalDevice, memRequirements.memoryTypeBits,
                                          vk::MemoryPropertyFlagBits::eHostVisible |
                                          vk::MemoryPropertyFlagBits::eHostCoherent),
    };

    memory = info.device.allocateMemory(allocInfo, nullptr);
    if (!memory)
    {
        fatalError("failed to allocate vertex buffer memory!");
    }

    info.device.bindBufferMemory(buffer, memory, 0);

    return result;
}

std::tuple<vk::Buffer, vk::DeviceMemory> RenderUtils::createDataBuffer(const void* data, vk::DeviceSize size,
                                                                       const CreateDataBufferInfo& info)
{
    auto allFamilyIndices = QueueFamilyUtils::findQueueFamilies(info.physicalDevice, info.surface);
    const auto familyIndicesToUse = {
        *allFamilyIndices.get(QueueFamilyType::Graphics), *allFamilyIndices.get(QueueFamilyType::Transfer)
    };

    const CreateBufferInfo stagingBufferInfo
    {
        .device = info.device,
        .physicalDevice = info.physicalDevice,
        .size = size,
        .usage = vk::BufferUsageFlagBits::eTransferSrc,
        .properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        .queueFamilyIndices = familyIndicesToUse,
    };

    auto [stagingBuffer, stagingBufferMemory] = createBuffer(stagingBufferInfo);

    void* mapData = info.device.mapMemory(stagingBufferMemory, 0, size);
    std::memcpy(mapData, data, size);
    info.device.unmapMemory(stagingBufferMemory);

    const CreateBufferInfo bufferInfo
    {
        .device = info.device,
        .physicalDevice = info.physicalDevice,
        .size = size,
        .usage = vk::BufferUsageFlagBits::eTransferDst | static_cast<vk::BufferUsageFlags>(info.usageType),
        .properties = vk::MemoryPropertyFlagBits::eDeviceLocal,
        .queueFamilyIndices = familyIndicesToUse,
    };

    auto result = createBuffer(bufferInfo);

    copyBuffer(info.device, info.transferCommandPool, info.transferQueue, size, stagingBuffer,
               std::get<vk::Buffer>(result));
    info.device.destroyBuffer(stagingBuffer);
    info.device.freeMemory(stagingBufferMemory);

    return result;
}

void RenderUtils::copyBuffer(vk::Device device, vk::CommandPool commandPool, vk::Queue queue, vk::DeviceSize size,
                             vk::Buffer srcBuffer, vk::Buffer dstBuffer)
{
    const vk::CommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

    vk::BufferCopy copyRegion{};
    copyRegion.size = size;
    commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);
    endSingleTimeCommands(device, commandBuffer, queue, commandPool);
}

bool RenderUtils::checkDeviceExtensionSupport(vk::PhysicalDevice device)
{
    const std::vector<vk::ExtensionProperties> availableExtensions = device.enumerateDeviceExtensionProperties(nullptr);
    const std::set<std::string> requiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

    auto available = [&](const char* requiredExtName)
    {
        auto matchesRequiredName = [&](vk::ExtensionProperties availableExt)
        {
            return std::strcmp(availableExt.extensionName, requiredExtName) == 0;
        };
        return std::ranges::any_of(availableExtensions, matchesRequiredName);
    };

    return std::ranges::all_of(DeviceExtensions, available);
}

namespace RenderUtils
{
    vk::Bool32 debugCallback
    (
        [[maybe_unused]] vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        [[maybe_unused]] vk::DebugUtilsMessageTypeFlagBitsEXT messageType,
        const vk::DebugUtilsMessengerCallbackDataEXT* callbackData,
        [[maybe_unused]] void* userData
    )
    {
        std::cerr << "validation layer: " << callbackData->pMessage << std::endl;
        return vk::False;
    }
}

vk::DebugUtilsMessengerCreateInfoEXT RenderUtils::newDebugUtilsMessengerCreateInfo()
{
    vk::DebugUtilsMessengerCreateInfoEXT info
    {
        .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        .pfnUserCallback = reinterpret_cast<PFN_vkDebugUtilsMessengerCallbackEXT>(debugCallback),
    };

    return info;
}

void RenderUtils::createDebugUtilsMessenger(vk::Instance instance, vk::DebugUtilsMessengerEXT* pDebugMessenger,
                                            const vk::AllocationCallbacks* pAllocator)
{
    if constexpr (!vk::EnableValidationLayers)
        return;

    const vk::DebugUtilsMessengerCreateInfoEXT createInfo = newDebugUtilsMessengerCreateInfo();
    *pDebugMessenger = instance.createDebugUtilsMessengerEXT(createInfo, pAllocator);
}

void RenderUtils::destroyDebugUtilsMessenger(vk::Instance instance, vk::DebugUtilsMessengerEXT debugMessenger,
                                             const vk::AllocationCallbacks* pAllocator)
{
    instance.destroyDebugUtilsMessengerEXT(debugMessenger, pAllocator);
}

RenderUtils::SwapChainSupportDetails RenderUtils::querySwapChainSupport(vk::PhysicalDevice device,
                                                                        vk::SurfaceKHR surface)
{
    return
    {
        .capabilities = device.getSurfaceCapabilitiesKHR(surface),
        .formats = device.getSurfaceFormatsKHR(surface),
        .presentModes = device.getSurfacePresentModesKHR(surface),
    };
}

vk::SurfaceFormatKHR RenderUtils::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
    check(!availableFormats.empty(), "No available surface formats!");

    auto isDesirableFormat = [](auto&& fmt)
    {
        return fmt.format == vk::Format::eB8G8R8A8Srgb && fmt.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
    };
    if (auto it = std::ranges::find_if(availableFormats, isDesirableFormat); it != availableFormats.end())
        return *it;

    return *availableFormats.begin();
}

vk::PresentModeKHR RenderUtils::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
    check(!availablePresentModes.empty(), "No available present modes!");

    // Disabling Mailbox for now as it doesn't seem to wait for vertical sync and instead goes as fast as it can
    // if (auto it = std::ranges::find(availablePresentModes, vk::PresentModeKHR::eMailbox); it != availablePresentModes.
    //     end())
    //     return *it;

    check(std::ranges::contains(availablePresentModes, vk::PresentModeKHR::eFifo), "Couldn't find required present mode: 'eFifo'");
    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D RenderUtils::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
{
    if (capabilities.currentExtent.width != std::numeric_limits<UInt32>::max())
    {
        return capabilities.currentExtent;
    }

    int width{}, height{};
    glfwGetFramebufferSize(window, &width, &height);

    vk::Extent2D actualExtent
    {
        static_cast<UInt32>(width),
        static_cast<UInt32>(height)
    };

    actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                                    capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                                     capabilities.maxImageExtent.height);

    return actualExtent;
}

std::vector<char> RenderUtils::readFile(const std::string& filename)
{
    std::cout << "Reading file at: " << filename << std::endl;
    std::ifstream file{filename, std::ios::ate | std::ios::binary};
    if (!file.is_open())
    {
        fatalError("failed to open file!");
    }

    const int fileSize = static_cast<int>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

vk::ShaderModule RenderUtils::createShaderModule(const std::vector<char>& code, vk::Device device)
{
    const vk::ShaderModuleCreateInfo createInfo
    {
        .codeSize = code.size(),
        .pCode = reinterpret_cast<const UInt32*>(code.data()),
    };

    return device.createShaderModule(createInfo, nullptr);
}

void RenderUtils::transitionImageLayout(vk::Device device, vk::Queue commandQueue, vk::CommandPool commandPool,
                                        vk::Image image, vk::Format format, vk::ImageLayout oldLayout,
                                        vk::ImageLayout newLayout)
{
    const vk::CommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

    vk::ImageAspectFlags aspectFlags;
    if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) // NOLINT(bugprone-branch-clone)
    {
        aspectFlags = vk::ImageAspectFlagBits::eDepth;

        if (hasStencilComponent(format))
            aspectFlags |= vk::ImageAspectFlagBits::eStencil;
    }
    else
    {
        aspectFlags = vk::ImageAspectFlagBits::eColor;
    }

    const struct Flags
    {
        vk::AccessFlags srcAccessMask;
        vk::AccessFlags dstAccessMask;
        vk::PipelineStageFlags srcStage;
        vk::PipelineStageFlags dstStage;
    } flags = [oldLayout, newLayout]() -> Flags
        {
            if (oldLayout == vk::ImageLayout::eUndefined
                && newLayout == vk::ImageLayout::eTransferDstOptimal)
                return
                {
                    .srcAccessMask = {},
                    .dstAccessMask = vk::AccessFlagBits::eTransferWrite,
                    .srcStage = vk::PipelineStageFlagBits::eTopOfPipe,
                    .dstStage = vk::PipelineStageFlagBits::eTransfer,
                };

            if (oldLayout == vk::ImageLayout::eTransferDstOptimal
                && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
                return
                {
                    .srcAccessMask = vk::AccessFlagBits::eTransferWrite,
                    .dstAccessMask = vk::AccessFlagBits::eShaderRead,
                    .srcStage = vk::PipelineStageFlagBits::eTransfer,
                    .dstStage = vk::PipelineStageFlagBits::eFragmentShader,
                };

            if (oldLayout == vk::ImageLayout::eUndefined
                && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
                return
                {
                    .srcAccessMask = {},
                    .dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead |
                    vk::AccessFlagBits::eDepthStencilAttachmentWrite,
                    .srcStage = vk::PipelineStageFlagBits::eTopOfPipe,
                    .dstStage = vk::PipelineStageFlagBits::eEarlyFragmentTests,
                };

            fatalError("unsupported layout transition!");
            return {};
        }();

    const vk::ImageMemoryBarrier barrier
    {
        .srcAccessMask = flags.srcAccessMask,
        .dstAccessMask = flags.dstAccessMask,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
        .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
        .image = image,
        .subresourceRange
        {
            .aspectMask = aspectFlags,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
    };

    commandBuffer.pipelineBarrier
    (
        flags.srcStage,
        flags.dstStage,
        {},
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    endSingleTimeCommands(device, commandBuffer, commandQueue, commandPool);
}

void RenderUtils::copyBufferToImage(vk::Device device, vk::Queue commandQueue, vk::CommandPool commandPool,
                                    vk::Buffer buffer, vk::Image image,
                                    vk::Extent2D extent)
{
    const vk::CommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

    const vk::BufferImageCopy region
    {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource
        {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
        .imageOffset = {0, 0, 0},
        .imageExtent
        {
            extent.width,
            extent.height,
            1
        },
    };

    commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);
    endSingleTimeCommands(device, commandBuffer, commandQueue, commandPool);
}

vk::Format RenderUtils::findSupportedFormat(vk::PhysicalDevice physicalDevice,
                                            const std::vector<vk::Format>& candidates, vk::ImageTiling tiling,
                                            vk::FormatFeatureFlags features)
{
    for (vk::Format format : candidates)
    {
        const vk::FormatProperties props = physicalDevice.getFormatProperties(format);

        if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features)
            return format;
        if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features)
            return format;
    }

    fatalError("failed to find supported format!");
    return {};
}

vk::Format RenderUtils::findDepthFormat(vk::PhysicalDevice physicalDevice)
{
    return findSupportedFormat
    (
        physicalDevice,
        {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment
    );
}
