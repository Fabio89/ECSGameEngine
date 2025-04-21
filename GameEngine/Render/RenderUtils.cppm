export module Render.Utils;
import Core;
import Wrapper.Glfw;
import Wrapper.Vulkan;
import Wrapper.Windows;
import vulkan_hpp;

export namespace RenderUtils
{
    constexpr auto ValidationLayers = std::to_array
    ({
        "VK_LAYER_KHRONOS_validation"
    });

    constexpr auto DeviceExtensions = std::to_array
    ({
        vk::KHRSwapchainExtensionName,
        vk::KHRDynamicRenderingExtensionName
    });

    UInt32 findMemoryType(vk::PhysicalDevice physicalDevice, UInt32 typeFilter, vk::MemoryPropertyFlags properties);

    vk::CommandBuffer beginSingleTimeCommands(vk::Device device, vk::CommandPool commandPool);
    void endSingleTimeCommands(vk::Device device, vk::CommandBuffer buffer, vk::Queue queue, vk::CommandPool pool);

    struct CreateBufferInfo
    {
        vk::Device device;
        vk::PhysicalDevice physicalDevice;
        vk::DeviceSize size;
        vk::BufferUsageFlags usage;
        vk::MemoryPropertyFlags properties;
        std::initializer_list<UInt32> queueFamilyIndices;
    };

    [[nodiscard]] std::tuple<vk::Buffer, vk::DeviceMemory> createBuffer(const CreateBufferInfo& info);

    struct CreateDataBufferInfo
    {
        vk::Device device;
        vk::PhysicalDevice physicalDevice;
        vk::SurfaceKHR surface;
        vk::BufferUsageFlagBits usageType;
        vk::Queue transferQueue;
        vk::CommandPool transferCommandPool;
    };

    template <typename T>
    concept BufferableData = requires { typename T::value_type; }
        && requires(const T& t) { { t.data() } -> std::convertible_to<const void*>; }
        && requires(const T& t) { { t.size() } -> std::integral; };

    template <BufferableData T>
    std::tuple<vk::Buffer, vk::DeviceMemory> createDataBuffer(const T& range, const CreateDataBufferInfo& info);
    std::tuple<vk::Buffer, vk::DeviceMemory> createDataBuffer(const void* data, vk::DeviceSize size, const CreateDataBufferInfo& info);

    template <BufferableData T>
    void updateBuffer(const T& range, vk::Device device, vk::DeviceMemory bufferMemory);
    
    void copyBuffer(vk::Device device, vk::CommandPool commandPool, vk::Queue queue, vk::DeviceSize size, vk::Buffer srcBuffer, vk::Buffer dstBuffer);

    [[nodiscard]] UInt32 findMemoryType(vk::PhysicalDevice physicalDevice, UInt32 typeFilter, vk::MemoryPropertyFlags properties);

    [[nodiscard]] std::tuple<vk::Buffer, vk::DeviceMemory> createBuffer(const CreateBufferInfo& info);

    template <BufferableData T>
    [[nodiscard]] std::tuple<vk::Buffer, vk::DeviceMemory> createDataBuffer(const T& range, const CreateDataBufferInfo& info)
    {
        return createDataBuffer(range.data(), sizeof(std::remove_reference_t<decltype(range)>::value_type) * range.size(), info);
    }

    bool checkDeviceExtensionSupport(vk::PhysicalDevice device);

    [[nodiscard]] vk::DebugUtilsMessengerCreateInfoEXT newDebugUtilsMessengerCreateInfo();

    void createDebugUtilsMessenger(vk::Instance instance, vk::DebugUtilsMessengerEXT* pDebugMessenger, const vk::AllocationCallbacks* pAllocator);

    void destroyDebugUtilsMessenger(vk::Instance instance, vk::DebugUtilsMessengerEXT debugMessenger, const vk::AllocationCallbacks* pAllocator);

    struct SwapChainSupportDetails
    {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };

    [[nodiscard]] SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device, vk::SurfaceKHR surface);

    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);

    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

    std::vector<char> readFile(const std::string& filename);
    vk::ShaderModule createShaderModule(const std::vector<char>& code, vk::Device device);

    void transitionImageLayout(vk::Device device, vk::Queue commandQueue, vk::CommandPool commandPool, vk::Image image, vk::Format format, vk::ImageLayout oldLayout,
                               vk::ImageLayout newLayout);

    void copyBufferToImage(vk::Device device, vk::Queue commandQueue, vk::CommandPool commandPool, vk::Buffer buffer, vk::Image image, vk::Extent2D extent);

    vk::Format findSupportedFormat(vk::PhysicalDevice physicalDevice, const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

    vk::Format findDepthFormat(vk::PhysicalDevice physicalDevice);

    void transitionImageLayout
    (
        vk::CommandBuffer cmd,
        vk::Image image,
        vk::ImageLayout oldLayout,
        vk::ImageLayout newLayout,
        vk::AccessFlags srcAccessMask,
        vk::AccessFlags dstAccessMask,
        vk::PipelineStageFlags srcStage,
        vk::PipelineStageFlags dstStage,
        vk::ImageAspectFlags aspectMask = vk::ImageAspectFlagBits::eColor
    );
    
    constexpr bool hasStencilComponent(vk::Format format)
    {
        return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
    }

    const std::string& getExecutableRoot()
    {
        static const std::string exeRoot = []
        {
            const auto moduleName = Wrapper_Windows::getModuleFileName();
            const auto pos = moduleName.find_last_of("\\/") + 1;
            return std::filesystem::canonical(moduleName.substr(0, pos)).generic_string() + "/";
        }();
        return exeRoot;
    }
}

template <RenderUtils::BufferableData T>
void RenderUtils::updateBuffer(const T& range, vk::Device device, vk::DeviceMemory bufferMemory)
{
    void* data;
    const size_t bufferSize = sizeof(std::remove_reference_t<decltype(range)>::value_type) * range.size();
    if (device.mapMemory(bufferMemory, 0, bufferSize, {}, &data) == vk::Result::eSuccess)
        std::memcpy(data, range.data(), bufferSize);        
    device.unmapMemory(bufferMemory);
}
