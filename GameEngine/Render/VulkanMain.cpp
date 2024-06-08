#include "VulkanMain.h"

bool checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

	auto available = [&](const char* requiredExtName)
	{
		auto matchesRequiredName = [&](VkExtensionProperties availableExt) { return strcmp(availableExt.extensionName, requiredExtName) == 0; };
		return std::ranges::any_of(availableExtensions, matchesRequiredName);
	};

	return std::ranges::all_of(DeviceExtensions, available);
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	if (auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")))
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	if (auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"))
	{
		func(instance, debugMessenger, pAllocator);
	}
}

bool areAllIndicesSet(const QueueFamilyIndices& indices)
{
	return indices.graphicsFamily.has_value()
		&& indices.presentFamily.has_value();
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	for (uint32_t i = 0; i < queueFamilies.size(); ++i)
	{
		const VkQueueFamilyProperties& family = queueFamilies[i];
		if ((family.queueFlags & VK_QUEUE_GRAPHICS_BIT))
			indices.graphicsFamily = i;

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if (presentSupport)
			indices.presentFamily = i;

		if (areAllIndicesSet(indices))
			break;
	}

	auto isGraphicsFamily = [](const VkQueueFamilyProperties& family) { return (family.queueFlags & VK_QUEUE_GRAPHICS_BIT); };
	if (auto it = std::ranges::find_if(queueFamilies, isGraphicsFamily); it != queueFamilies.end())
	{
		indices.graphicsFamily = static_cast<uint32_t>(it - queueFamilies.begin());
	}

	return indices;
}

VKAPI_ATTR VkBool32 VKAPI_CALL HelloTriangleApplication::debugCallback
(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData
)
{
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}

void HelloTriangleApplication::run()
{
	// Init window
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		m_window = glfwCreateWindow(m_windowSize.x, m_windowSize.y, "Vulkan", nullptr, nullptr);
	}

	// Init Vulkan
	{
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
	}

	// Main loop
	while (!glfwWindowShouldClose(m_window))
	{
		glfwPollEvents();
	}

	// Cleanup
	{
		if (EnableValidationLayers)
		{
			DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
		}

		vkDestroyDevice(m_device, nullptr);
		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
		vkDestroyInstance(m_instance, nullptr);
		glfwDestroyWindow(m_window);
		glfwTerminate();
	}
}

VkDebugUtilsMessengerCreateInfoEXT HelloTriangleApplication::newDebugUtilsMessengerCreateInfo()
{
	return
	{
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = debugCallback,
		.pUserData = nullptr, // Optional
	};
}

void HelloTriangleApplication::createInstance()
{
	if (EnableValidationLayers && !checkValidationLayerSupport())
		throw std::runtime_error("validation layers requested, but not available!");

	const VkApplicationInfo appInfo
	{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "Hello Triangle",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "No Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_0,
	};

	auto extensions = getRequiredExtensions();

	VkInstanceCreateInfo createInfo
	{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = nullptr,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = 0,
		.enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
		.ppEnabledExtensionNames = extensions.data(),
	};

	const auto debugCreateInfo = newDebugUtilsMessengerCreateInfo();

	if (EnableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
		createInfo.ppEnabledLayerNames = ValidationLayers.data();
		createInfo.pNext = &debugCreateInfo;
	}

	if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
}

void HelloTriangleApplication::createSurface()
{
	if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS)
		throw std::runtime_error("failed to create window surface!");
}

void HelloTriangleApplication::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice, m_surface);

	std::set<uint32_t> uniqueQueueFamilies{ indices.graphicsFamily.value(), indices.presentFamily.value() };
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	float queuePriority = 1.0f;

	queueCreateInfos.reserve(uniqueQueueFamilies.size());

	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		queueCreateInfos.emplace_back(VkDeviceQueueCreateInfo
			{
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = queueFamily,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority,
			});
	}

	const VkPhysicalDeviceFeatures deviceFeatures{};

	const VkDeviceCreateInfo createInfo
	{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
		.pQueueCreateInfos = queueCreateInfos.data(),
		.enabledLayerCount = [&] { if constexpr (EnableValidationLayers) return static_cast<uint32_t>(ValidationLayers.size()); else return 0; }(),
		.ppEnabledLayerNames = [&] { if constexpr (EnableValidationLayers) return ValidationLayers.data(); else return nullptr; }(),
		.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size()),
		.ppEnabledExtensionNames = DeviceExtensions.data(),
		.pEnabledFeatures = &deviceFeatures,
	};

	if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
		throw std::runtime_error("failed to create logical device!");

	vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
}

void HelloTriangleApplication::setupDebugMessenger()
{
	if (!EnableValidationLayers)
		return;

	const auto createInfo = newDebugUtilsMessengerCreateInfo();

	if (CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

void HelloTriangleApplication::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

	if (deviceCount == 0)
		throw std::runtime_error("failed to find GPUs with Vulkan support!");

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

	auto isDeviceSuitable = [&](VkPhysicalDevice device)
	{
		return areAllIndicesSet(findQueueFamilies(device, m_surface))
			&& checkDeviceExtensionSupport(device);
	};

	auto found = std::ranges::find_if(devices, isDeviceSuitable);

	if (found != devices.end())
		m_physicalDevice = *found;
	else
		throw std::runtime_error("failed to find a suitable GPU!");
}

bool HelloTriangleApplication::checkValidationLayerSupport() const
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	auto isLayerAvailable = [&](const char* layerName) {
		auto matchesName = [&](const VkLayerProperties& item) { return strcmp(layerName, item.layerName) == 0; };
		return std::ranges::any_of(availableLayers, matchesName); };

	return std::ranges::all_of(ValidationLayers, isLayerAvailable);
}

std::vector<const char*> HelloTriangleApplication::getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (EnableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

int main()
{
	HelloTriangleApplication app;

	try
	{
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}

	return 0;
}