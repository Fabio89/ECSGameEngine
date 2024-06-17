export module Engine.Render.Core:Utils;
import :Vulkan;
import std;

export namespace RenderUtils
{
    const std::vector ValidationLayers
    {
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector DeviceExtensions
    {
        vk::KHRSwapchainExtensionName
    };

    uint32_t findMemoryType(vk::PhysicalDevice physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties);

    vk::CommandBuffer beginSingleTimeCommands(vk::Device device, vk::CommandPool commandPool);
    void endSingleTimeCommands(vk::Device device, vk::CommandBuffer buffer, vk::Queue queue, vk::CommandPool pool);

    struct CreateBufferInfo
    {
        vk::Device device;
        vk::PhysicalDevice physicalDevice;
        vk::DeviceSize size;
        vk::BufferUsageFlags usage;
        vk::MemoryPropertyFlags properties;
        std::initializer_list<uint32_t> queueFamilyIndices;
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
    concept bufferable_data = requires { typename T::value_type; }
        && requires(const T& t) { { t.data() } -> std::convertible_to<const void*>; }
        && requires(const T& t) { { t.size() } -> std::integral; };

    template <bufferable_data T>
    std::tuple<vk::Buffer, vk::DeviceMemory> createDataBuffer(const T& range, const CreateDataBufferInfo& info);
    std::tuple<vk::Buffer, vk::DeviceMemory> createDataBuffer(const void* data, vk::DeviceSize size,
                                                              const CreateDataBufferInfo& info);

    void copyBuffer(vk::Device device, vk::CommandPool commandPool, vk::Queue queue, vk::DeviceSize size,
                    vk::Buffer srcBuffer, vk::Buffer dstBuffer);


    [[nodiscard]] uint32_t findMemoryType(vk::PhysicalDevice physicalDevice, uint32_t typeFilter,
                                          vk::MemoryPropertyFlags properties);

    [[nodiscard]] std::tuple<vk::Buffer, vk::DeviceMemory> createBuffer(const CreateBufferInfo& info);

    template <bufferable_data T>
    [[nodiscard]] std::tuple<vk::Buffer, vk::DeviceMemory> createDataBuffer(
        const T& range, const CreateDataBufferInfo& info)
    {
        return createDataBuffer(range.data(),
                                sizeof(std::remove_reference<decltype(range)>::type::value_type) * range.size(), info);
    }

    bool checkDeviceExtensionSupport(vk::PhysicalDevice device);

    [[nodiscard]] vk::DebugUtilsMessengerCreateInfoEXT newDebugUtilsMessengerCreateInfo();

    void createDebugUtilsMessenger(vk::Instance instance, vk::DebugUtilsMessengerEXT* pDebugMessenger,
                                   const vk::AllocationCallbacks* pAllocator);

    void destroyDebugUtilsMessenger(vk::Instance instance, vk::DebugUtilsMessengerEXT debugMessenger,
                                    const vk::AllocationCallbacks* pAllocator);

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

    void transitionImageLayout(vk::Device device, vk::Queue commandQueue, vk::CommandPool commandPool, vk::Image image,
                               vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

    void copyBufferToImage(vk::Device device, vk::Queue commandQueue, vk::CommandPool commandPool, vk::Buffer buffer, vk::Image image,
    vk::Extent2D extent);
}
