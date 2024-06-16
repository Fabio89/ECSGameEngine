module;

#include <cassert>
#include <vulkan/vk_platform.h>

module Engine.Render.Core;

[[nodiscard]] uint32_t RenderUtils::findMemoryType(vk::PhysicalDevice physicalDevice, uint32_t typeFilter,
                                                   vk::MemoryPropertyFlags properties)
{
    vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
    {
        if ((typeFilter & (1 << i))
            && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
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

void RenderUtils::endSingleTimeCommands(vk::CommandBuffer commandBuffer, vk::Device device, vk::Queue queue,
                                        vk::CommandPool pool)
{
    commandBuffer.end();
    const vk::SubmitInfo submitInfo
    {
        . commandBufferCount = 1,
        . pCommandBuffers = &commandBuffer,
    };

    // ReSharper disable once CppDeclaratorNeverUsed
    auto result = queue.submit(1, &submitInfo, nullptr);
    queue.waitIdle();
    device.freeCommandBuffers(pool, 1, &commandBuffer);
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
        .queueFamilyIndexCount = static_cast<uint32_t>(info.queueFamilyIndices.size()),
        .pQueueFamilyIndices = info.queueFamilyIndices.begin()
    };

    buffer = info.device.createBuffer(bufferInfo, nullptr);
    if (!buffer)
    {
        throw std::runtime_error("failed to create vertex buffer!");
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
        throw std::runtime_error("failed to allocate vertex buffer memory!");
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
    memcpy(mapData, data, size);
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
    endSingleTimeCommands(commandBuffer, device, queue, commandPool);
}

bool RenderUtils::checkDeviceExtensionSupport(vk::PhysicalDevice device)
{
    const std::vector<vk::ExtensionProperties> availableExtensions = device.enumerateDeviceExtensionProperties(nullptr);
    const std::set<std::string> requiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

    auto available = [&](const char* requiredExtName)
    {
        auto matchesRequiredName = [&](vk::ExtensionProperties availableExt)
        {
            return strcmp(availableExt.extensionName, requiredExtName) == 0;
        };
        return std::ranges::any_of(availableExtensions, matchesRequiredName);
    };

    return std::ranges::all_of(DeviceExtensions, available);
}

namespace RenderUtils
{
    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback
    (
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    )
    {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        return vk::False;
    }
}

vk::DebugUtilsMessengerCreateInfoEXT RenderUtils::newDebugUtilsMessengerCreateInfo()
{
    vk::DebugUtilsMessengerCreateInfoEXT info{};
    info.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
    info.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        info.pfnUserCallback = debugCallback;
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

vk::SurfaceFormatKHR RenderUtils::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
    assert(!availableFormats.empty());

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
    assert(!availablePresentModes.empty());

    if (auto it = std::ranges::find(availablePresentModes, vk::PresentModeKHR::eMailbox); it != availablePresentModes.
        end())
        return *it;

    assert(std::ranges::contains(availablePresentModes, vk::PresentModeKHR::eFifo));
    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D RenderUtils::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        vk::Extent2D actualExtent
        {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                                        capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                                         capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

std::vector<char> RenderUtils::readFile(const std::string& filename)
{
    std::ifstream file{filename, std::ios::ate | std::ios::binary};
    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file!");
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
        .pCode = reinterpret_cast<const uint32_t*>(code.data()),
    };

    return device.createShaderModule(createInfo, nullptr);
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
