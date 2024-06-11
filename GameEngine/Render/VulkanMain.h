#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

import std.core;
import <glm/ext/vector_int2.hpp>;

const std::vector<const char*> ValidationLayers
{
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> DeviceExtensions
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

bool checkDeviceExtensionSupport(VkPhysicalDevice device);

static constexpr bool EnableValidationLayers =
#if NDEBUG
false;
#else
true;
#endif

VkDebugUtilsMessengerCreateInfoEXT newDebugUtilsMessengerCreateInfo();
void createDebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* pDebugMessenger, const VkAllocationCallbacks* pAllocator);
void destroyDebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
};
bool areAllIndicesSet(const QueueFamilyIndices&);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

std::vector<char> readFile(const std::string& filename);
VkShaderModule createShaderModule(const std::vector<char>& code, VkDevice device);

struct ImGui_ImplVulkan_InitInfo;
class ImGuiHelper
{
public:
	void init(GLFWwindow* window, ImGui_ImplVulkan_InitInfo& initInfo);
	void drawFrame();
	void renderFrame(VkCommandBuffer commandBuffer);
	void shutdown();

private:
	bool m_showDemoWindow{ true };
};

class HelloTriangleApplication
{
public:
	void init();
	void update();
	void shutdown();
	bool shouldWindowClose() const;

private:
	glm::ivec2 m_windowSize{ 1600, 900 };
	GLFWwindow* m_window{ nullptr };
	VkInstance m_instance{ nullptr };
	VkPhysicalDevice m_physicalDevice{ nullptr };
	VkDevice m_device{ nullptr };
	VkSurfaceKHR m_surface{ nullptr };
	VkSwapchainKHR m_swapChain{ nullptr };
	VkQueue m_graphicsQueue{ nullptr };
	VkQueue m_presentQueue{ nullptr };
	VkRenderPass m_renderPass{ nullptr };
	VkPipelineLayout m_pipelineLayout{ nullptr };
	VkPipeline m_graphicsPipeline{ nullptr };

	VkCommandPool m_commandPool{ nullptr };
	VkCommandBuffer m_commandBuffer{ nullptr };

	VkPipelineCache m_pipelineCache{ nullptr };
	VkDescriptorPool m_descriptorPool{ nullptr };

	std::vector<VkImage> m_swapChainImages;
	std::vector<VkImageView> m_swapChainImageViews;
	std::vector<VkFramebuffer> m_swapChainFramebuffers;
	VkFormat m_swapChainImageFormat{ VK_FORMAT_UNDEFINED };
	VkExtent2D m_swapChainExtent{ 0, 0 };

	VkSemaphore m_imageAvailableSemaphore{ nullptr };
	VkSemaphore m_renderFinishedSemaphore{ nullptr };
	VkFence m_inFlightFence{ nullptr };

	VkDebugUtilsMessengerEXT m_debugMessenger{ nullptr };

	ImGuiHelper imguiHelper;

	static std::vector<const char*> getRequiredExtensions();

	void createInstance();
	void createSurface();
	void createLogicalDevice();
	void createSwapChain();
	void createImageViews();
	void createRenderPass();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffer();
	void createSyncObjects();
	void createDescriptorPool();
	void initImguiHelper();
	void pickPhysicalDevice();
	bool checkValidationLayerSupport() const;
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void drawFrame();
};