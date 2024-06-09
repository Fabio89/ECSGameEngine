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

constexpr bool EnableValidationLayers =
#if NDEBUG
false;
#else
true;
#endif

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

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

class HelloTriangleApplication
{
public:
	void run();

private:
	glm::ivec2 m_windowSize{ 900, 600 };
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
	VkDebugUtilsMessengerEXT m_debugMessenger{ nullptr };
	std::vector<VkImage> m_swapChainImages;
	std::vector<VkImageView> m_swapChainImageViews;
	VkFormat m_swapChainImageFormat{ VK_FORMAT_UNDEFINED };
	VkExtent2D m_swapChainExtent{ 0, 0 };

	static VkDebugUtilsMessengerCreateInfoEXT newDebugUtilsMessengerCreateInfo();

	static std::vector<const char*> getRequiredExtensions();

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback
	(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	);

	void createInstance();
	void createSurface();
	void createLogicalDevice();
	void createSwapChain();
	void createImageViews();
	void createRenderPass();
	void createGraphicsPipeline();
	void setupDebugMessenger();
	void pickPhysicalDevice();
	bool checkValidationLayerSupport() const;
};