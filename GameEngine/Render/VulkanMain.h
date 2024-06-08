#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/ext/vector_int2.hpp>

import std.core;

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
	VkQueue m_graphicsQueue{ nullptr };
	VkQueue m_presentQueue{ nullptr };
	VkDebugUtilsMessengerEXT m_debugMessenger{ nullptr };

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
	void setupDebugMessenger();
	void pickPhysicalDevice();
	bool checkValidationLayerSupport() const;
};