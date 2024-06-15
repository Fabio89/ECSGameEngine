#include "VulkanMain.h"

import <imgui.h>;
import <imgui_impl_glfw.h>;
import <imgui_impl_vulkan.h>;

#define GLM_FORCE_RADIANS  // NOLINT(clang-diagnostic-unused-macros)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

bool checkDeviceExtensionSupport(vk::PhysicalDevice device)
{
	const std::vector<vk::ExtensionProperties> availableExtensions = device.enumerateDeviceExtensionProperties(nullptr);
	const std::set<std::string> requiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

	auto available = [&](const char* requiredExtName)
	{
		auto matchesRequiredName = [&](vk::ExtensionProperties availableExt) { return strcmp(availableExt.extensionName, requiredExtName) == 0; };
		return std::ranges::any_of(availableExtensions, matchesRequiredName);
	};

	return std::ranges::all_of(DeviceExtensions, available);
}

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

vk::DebugUtilsMessengerCreateInfoEXT newDebugUtilsMessengerCreateInfo()
{
	return
	{
		.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
		.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
		.pfnUserCallback = debugCallback,
		.pUserData = nullptr, // Optional
	};
}

void createDebugUtilsMessenger(vk::Instance instance, vk::DebugUtilsMessengerEXT* pDebugMessenger, const vk::AllocationCallbacks* pAllocator)
{
	if constexpr (!vk::EnableValidationLayers)
		return;

	const vk::DebugUtilsMessengerCreateInfoEXT createInfo = newDebugUtilsMessengerCreateInfo();	
	*pDebugMessenger = instance.createDebugUtilsMessengerEXT(createInfo, pAllocator);
}

void destroyDebugUtilsMessenger(vk::Instance instance, vk::DebugUtilsMessengerEXT debugMessenger, const vk::AllocationCallbacks* pAllocator)
{
	instance.destroyDebugUtilsMessengerEXT(debugMessenger, pAllocator);
}

SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device, vk::SurfaceKHR surface)
{
	return
	{
		.capabilities = device.getSurfaceCapabilitiesKHR(surface),
		.formats = device.getSurfaceFormatsKHR(surface),
		.presentModes = device.getSurfacePresentModesKHR(surface),
	};
}

vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
	assert(!availableFormats.empty());

	auto isDesirableFormat = [](auto&& fmt) { return fmt.format == vk::Format::eB8G8R8A8Srgb && fmt.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; };
	if (auto it = std::ranges::find_if(availableFormats, isDesirableFormat); it != availableFormats.end())
		return *it;

	return *availableFormats.begin();
}

vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
	assert(!availablePresentModes.empty());

	if (auto it = std::ranges::find(availablePresentModes, vk::PresentModeKHR::eMailbox); it != availablePresentModes.end())
		return *it;

	assert(std::ranges::contains(availablePresentModes, vk::PresentModeKHR::eFifo));
	return vk::PresentModeKHR::eFifo;
}

vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
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

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

std::vector<char> readFile(const std::string& filename)
{
	std::ifstream file{ filename, std::ios::ate | std::ios::binary };
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

vk::ShaderModule createShaderModule(const std::vector<char>& code, vk::Device device)
{
	const vk::ShaderModuleCreateInfo createInfo
	{

		.codeSize = code.size(),
		.pCode = reinterpret_cast<const uint32_t*>(code.data()),
	};

	return device.createShaderModule(createInfo, nullptr);
}

void HelloTriangleApplication::init()
{
	// Init window
	{
		glfwInit();
		glfwWindowHint(glfw::ClientApi, glfw::NoApi);
		glfwWindowHint(glfw::Resizable, glfw::True);
		m_window = glfwCreateWindow(m_windowSize.x, m_windowSize.y, "Vulkan", nullptr, nullptr);
		glfwSetWindowUserPointer(m_window, this);
		glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
	}

	// Init Vulkan
	{
		vk::defaultDispatchLoaderDynamic.init();

		// Determine what API version is available
		const uint32_t apiVersion = vk::enumerateInstanceVersion();
		std::cout << "Loader/Runtime support detected for Vulkan " << VK_VERSION_MAJOR(apiVersion) << "." << VK_VERSION_MINOR(apiVersion) << "\n";

		createInstance();

		createDebugUtilsMessenger(m_instance, &m_debugMessenger, nullptr);
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapchain();
		createImageViews();
		createRenderPass();
		createDescriptorSetLayout();
		createGraphicsPipeline();
		createFramebuffers();
		createCommandPool();
		createVertexBuffer();
		createIndexBuffer();
		createUniformBuffers();
		createCommandBuffers();
		createSyncObjects();
		createDescriptorPool();
	}
	
	initImguiHelper();
}

void HelloTriangleApplication::update(float deltaTime)
{
	glfwPollEvents();
	drawFrame(deltaTime);
}

void HelloTriangleApplication::shutdown()
{
	vkDeviceWaitIdle(m_device);

	ImGuiHelper::shutdown();

	cleanupSwapchain();

	for (auto&& [buffer, memory] : std::views::zip(m_uniformBuffers, m_uniformBuffersMemory))
	{
		vkDestroyBuffer(m_device, buffer, nullptr);
		vkFreeMemory(m_device, memory, nullptr);
	}

	vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
	vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
	vkFreeMemory(m_device, m_vertexBufferMemory, nullptr);
	vkDestroyBuffer(m_device, m_indexBuffer, nullptr);
	vkFreeMemory(m_device, m_indexBufferMemory, nullptr);
	vkDestroyCommandPool(m_device, m_commandPool, nullptr);
	vkDestroyCommandPool(m_device, m_transferCommandPool, nullptr);
	vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
	vkDestroyRenderPass(m_device, m_renderPass, nullptr);

	for (size_t i = 0; i < MaxFramesInFlight; ++i)
	{
		vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
	}

	vkDestroyDevice(m_device, nullptr);
	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

	if (vk::EnableValidationLayers)
	{
		destroyDebugUtilsMessenger(m_instance, m_debugMessenger, nullptr);
	}

	vkDestroyInstance(m_instance, nullptr);
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

bool HelloTriangleApplication::shouldWindowClose() const
{
	return glfwWindowShouldClose(m_window);
}

void HelloTriangleApplication::createInstance()
{
	if constexpr (vk::EnableValidationLayers)
		if (!checkValidationLayerSupport())
			throw std::runtime_error("validation layers requested, but not available!");

	constexpr vk::ApplicationInfo appInfo
	{
		.pApplicationName = "Hello Triangle",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "No Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_3,
	};

	auto extensions = getRequiredExtensions();

	vk::InstanceCreateInfo createInfo
	{
		.pNext = nullptr,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = 0,
		.enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
		.ppEnabledExtensionNames = extensions.data(),
	};

	const auto debugCreateInfo = newDebugUtilsMessengerCreateInfo();

	if (vk::EnableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
		createInfo.ppEnabledLayerNames = ValidationLayers.data();
		createInfo.pNext = &debugCreateInfo;
	}

	m_instance = vk::createInstance(createInfo);

	vk::defaultDispatchLoaderDynamic.init(m_instance);
}

void HelloTriangleApplication::createSurface()
{
	if (glfw::createWindowSurface(m_instance, m_window, nullptr, &m_surface) != vk::Result::eSuccess)
		throw std::runtime_error("failed to create window surface!");
}

void HelloTriangleApplication::createLogicalDevice()
{
	const QueueFamilyIndices indices = QueueFamilyUtils::findQueueFamilies(m_physicalDevice, m_surface);

	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	float queuePriority = 1.0f;

	queueCreateInfos.reserve(indices.size());

	auto indexRange = indices | std::views::transform([&](auto&& index) { return *index; });
	std::set<uint32_t> uniqueQueueFamilies(indexRange.begin(), indexRange.end());
	
	for (std::optional<uint32_t> queueFamily : uniqueQueueFamilies)
	{
		assert(queueFamily.has_value());
		if (!queueFamily.has_value())
			continue;

		queueCreateInfos.emplace_back(vk::DeviceQueueCreateInfo
		{
			.queueFamilyIndex = *queueFamily,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority,
		});
	}

	constexpr vk::PhysicalDeviceFeatures deviceFeatures{};

	const vk::DeviceCreateInfo createInfo
	{
		.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
		.pQueueCreateInfos = queueCreateInfos.data(),
		.enabledLayerCount = [&] { if constexpr (vk::EnableValidationLayers) return static_cast<uint32_t>(ValidationLayers.size()); else return 0; }(),
		.ppEnabledLayerNames = [&] { if constexpr (vk::EnableValidationLayers) return ValidationLayers.data(); else return nullptr; }(),
		.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size()),
		.ppEnabledExtensionNames = DeviceExtensions.data(),
		.pEnabledFeatures = &deviceFeatures,
	};

	m_device = m_physicalDevice.createDevice(createInfo);
	if (!m_device)
		throw std::runtime_error("failed to create logical device!");

	m_graphicsQueue = m_device.getQueue(*indices.get(QueueFamilyType::Graphics), 0);
	m_presentQueue = m_device.getQueue(*indices.get(QueueFamilyType::Present), 0);
	m_transferQueue = m_device.getQueue(*indices.get(QueueFamilyType::Transfer), 0);
}

void HelloTriangleApplication::recreateSwapchain()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(m_window, &width, &height);
	while (width == 0 || height == 0)  // NOLINT(bugprone-infinite-loop)
	{
		glfwGetFramebufferSize(m_window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(m_device);

	cleanupSwapchain();

	createSwapchain();
	createImageViews();
	createFramebuffers();
}

void HelloTriangleApplication::createSwapchain()
{
	const SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_physicalDevice, m_surface);

	const vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	const vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	const vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities, m_window);

	uint32_t minImageCount = swapChainSupport.capabilities.minImageCount + 1;

	if (swapChainSupport.capabilities.maxImageCount > 0 && minImageCount > swapChainSupport.capabilities.maxImageCount) {
		minImageCount = swapChainSupport.capabilities.maxImageCount;
	}

	vk::SwapchainCreateInfoKHR createInfo
	{

		.surface = m_surface,
		.minImageCount = minImageCount,
		.imageFormat = surfaceFormat.format,
		.imageColorSpace = surfaceFormat.colorSpace,
		.imageExtent = extent,
		.imageArrayLayers = 1,
		.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
		.preTransform = swapChainSupport.capabilities.currentTransform,
		.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
		.presentMode = presentMode,
		.clipped = vk::False,
		.oldSwapchain = nullptr,
	};

	const QueueFamilyIndices indices = QueueFamilyUtils::findQueueFamilies(m_physicalDevice, m_surface);
	auto indicesRange = indices | std::views::transform([](std::optional<uint32_t> value) { return *value; });
	const std::vector<uint32_t> queueFamilyIndices(indicesRange.begin(), indicesRange.end());

	if (indices.get(QueueFamilyType::Graphics) != indices.get(QueueFamilyType::Present))
	{
		createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
	}
	else
	{
		createInfo.imageSharingMode = vk::SharingMode::eExclusive;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	m_swapChain = m_device.createSwapchainKHR(createInfo, nullptr);
	if (!m_swapChain)
	{
		throw std::runtime_error("failed to create swap chain!");
	}

	m_swapChainImages = m_device.getSwapchainImagesKHR(m_swapChain);
	m_swapChainImageFormat = surfaceFormat.format;
	m_swapChainExtent = extent;
}

void HelloTriangleApplication::createImageViews()
{
	m_swapChainImageViews.resize(m_swapChainImages.size());

	for (size_t i = 0; i < m_swapChainImages.size(); ++i)
	{
		const vk::ImageViewCreateInfo createInfo
		{

			.image = m_swapChainImages[i],
			.viewType = vk::ImageViewType::e2D,
			.format = m_swapChainImageFormat,
			.components
			{
				.r = vk::ComponentSwizzle::eIdentity,
				.g = vk::ComponentSwizzle::eIdentity,
				.b = vk::ComponentSwizzle::eIdentity,
				.a = vk::ComponentSwizzle::eIdentity
			},
			.subresourceRange
			{
				.aspectMask = vk::ImageAspectFlagBits::eColor,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};

		m_swapChainImageViews[i] = m_device.createImageView(createInfo, nullptr);
		if (!m_swapChainImageViews[i])
		{
			throw std::runtime_error("failed to create image views!");
		}

	}
}

void HelloTriangleApplication::createRenderPass()
{
	const vk::AttachmentDescription colorAttachment
	{
		.format = m_swapChainImageFormat,
		.samples = vk::SampleCountFlagBits::e1,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
		.stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
		.initialLayout = vk::ImageLayout::eUndefined,
		.finalLayout = vk::ImageLayout::ePresentSrcKHR,
	};

	constexpr vk::AttachmentReference colorAttachmentRef
	{
		.attachment = 0,
		.layout = vk::ImageLayout::eColorAttachmentOptimal,
	};

	const vk::SubpassDescription subpass
	{
		.pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentRef,
	};

	constexpr vk::SubpassDependency dependency
	{
		.srcSubpass = vk::SubpassExternal,
		.dstSubpass = 0,
		.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
		.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
		.srcAccessMask = {},
		.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
	};

	const vk::RenderPassCreateInfo renderPassInfo
	{
		.attachmentCount = 1,
		.pAttachments = &colorAttachment,
		.subpassCount = 1,
		.pSubpasses = &subpass,
		.dependencyCount = 1,
		.pDependencies = &dependency
	};

	m_renderPass = m_device.createRenderPass(renderPassInfo, nullptr);
	if (!m_renderPass)
	{
		throw std::runtime_error("failed to create render pass!");
	}
}

void HelloTriangleApplication::createDescriptorSetLayout()
{
	constexpr vk::DescriptorSetLayoutBinding layoutBinding
	{
		.binding = 0,
		.descriptorType = vk::DescriptorType::eUniformBuffer,
		.descriptorCount = 1,
		.stageFlags = vk::ShaderStageFlagBits::eVertex,
		.pImmutableSamplers = nullptr, // Optional
	};

	const vk::DescriptorSetLayoutCreateInfo layoutInfo
	{
		.bindingCount = 1,
		.pBindings = &layoutBinding,
	};

	m_descriptorSetLayout = m_device.createDescriptorSetLayout(layoutInfo, nullptr);
	if (!m_descriptorSetLayout)
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void HelloTriangleApplication::createGraphicsPipeline()
{
	auto vertShaderCode = readFile("D:/Dev/EcsGameEngine/GameEngine/Render/Shaders/vert.spv");
	auto fragShaderCode = readFile("D:/Dev/EcsGameEngine/GameEngine/Render/Shaders/frag.spv");

	vk::ShaderModule vertShaderModule = createShaderModule(vertShaderCode, m_device);
	vk::ShaderModule fragShaderModule = createShaderModule(fragShaderCode, m_device);

	const vk::PipelineShaderStageCreateInfo vertShaderStageInfo
	{
		.stage = vk::ShaderStageFlagBits::eVertex,
		.module = vertShaderModule,
		.pName = "main",
	};

	const vk::PipelineShaderStageCreateInfo fragShaderStageInfo
	{

		.stage = vk::ShaderStageFlagBits::eFragment,
		.module = fragShaderModule,
		.pName = "main",
	};

	const vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	const std::vector<vk::DynamicState> dynamicStates
	{
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor
	};

	const vk::PipelineDynamicStateCreateInfo dynamicStateInfo
	{
		.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
		.pDynamicStates = dynamicStates.data(),
	};

	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	const vk::PipelineVertexInputStateCreateInfo vertexInputInfo
	{
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &bindingDescription, // Optional
		.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
		.pVertexAttributeDescriptions = attributeDescriptions.data(), // Optional
	};

	constexpr vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo
	{
		.topology = vk::PrimitiveTopology::eTriangleList,
		.primitiveRestartEnable = vk::False,
	};

	const vk::Viewport viewport
	{
		.x = 0.0f,
		.y = 0.0f,
		.width = static_cast<float>(m_swapChainExtent.width),
		.height = static_cast<float>(m_swapChainExtent.height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};

	const vk::Rect2D scissor
	{
		.offset = { 0, 0 },
		.extent = m_swapChainExtent,
	};

	const vk::PipelineViewportStateCreateInfo viewportStateInfo
	{
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor
	};

	const vk::PipelineRasterizationStateCreateInfo rasterizerInfo
	{
		.depthClampEnable = vk::False,
		.rasterizerDiscardEnable = vk::False,
		.polygonMode = vk::PolygonMode::eFill,
		.cullMode = vk::CullModeFlagBits::eBack,
		.frontFace = vk::FrontFace::eClockwise,
		.depthBiasEnable = vk::False,
		.depthBiasConstantFactor = 0.0f, // Optional
		.depthBiasClamp = 0.0f, // Optional
		.depthBiasSlopeFactor = 0.0f, // Optional
		.lineWidth = 1.0f,
	};

	const vk::PipelineMultisampleStateCreateInfo multisamplingInfo
	{
		.rasterizationSamples = vk::SampleCountFlagBits::e1,
		.sampleShadingEnable = vk::False,
		.minSampleShading = 1.0f, // Optional
		.pSampleMask = nullptr, // Optional
		.alphaToCoverageEnable = vk::False, // Optional
		.alphaToOneEnable = vk::False, // Optional
	};

	const vk::PipelineColorBlendAttachmentState colorBlendAttachment
	{
		.blendEnable = vk::False,
		.srcColorBlendFactor = vk::BlendFactor::eOne, // Optional
		.dstColorBlendFactor = vk::BlendFactor::eZero, // Optional
		.colorBlendOp = vk::BlendOp::eAdd, // Optional
		.srcAlphaBlendFactor = vk::BlendFactor::eOne, // Optional
		.dstAlphaBlendFactor = vk::BlendFactor::eZero, // Optional
		.alphaBlendOp = vk::BlendOp::eAdd, // Optional
		.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
	};

	const vk::PipelineColorBlendStateCreateInfo colorBlendingInfo
	{
		.logicOpEnable = vk::False,
		.logicOp = vk::LogicOp::eCopy, // Optional
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment,
		.blendConstants = std::array<float, 4>{ 0.f, 0.f, 0.f, 0.f }
	};

	const vk::PipelineLayoutCreateInfo pipelineLayoutInfo
	{
		.setLayoutCount = 1,
		.pSetLayouts = &m_descriptorSetLayout,
		.pushConstantRangeCount = 0, // Optional
		.pPushConstantRanges = nullptr, // Optional
	};

	m_pipelineLayout = m_device.createPipelineLayout(pipelineLayoutInfo, nullptr);
	if (!m_pipelineLayout)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}

	const vk::GraphicsPipelineCreateInfo pipelineInfo
	{
		.stageCount = 2,
		.pStages = shaderStages,
		.pVertexInputState = &vertexInputInfo,
		.pInputAssemblyState = &inputAssemblyInfo,
		.pViewportState = &viewportStateInfo,
		.pRasterizationState = &rasterizerInfo,
		.pMultisampleState = &multisamplingInfo,
		.pDepthStencilState = nullptr, // Optional
		.pColorBlendState = &colorBlendingInfo,
		.pDynamicState = &dynamicStateInfo,
		.layout = m_pipelineLayout,
		.renderPass = m_renderPass,
		.subpass = 0,
		.basePipelineHandle = nullptr,
		.basePipelineIndex = -1,
	};

	auto&&[result, pipeline] = m_device.createGraphicsPipeline(vk::PipelineCache{}, pipelineInfo, nullptr);

	m_graphicsPipeline = pipeline;

	if (result != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	m_device.destroyShaderModule(fragShaderModule, nullptr);
	m_device.destroyShaderModule(vertShaderModule, nullptr);
}

void HelloTriangleApplication::createFramebuffers()
{
	m_swapChainFramebuffers.resize(m_swapChainImageViews.size());

	for (size_t i = 0; i < m_swapChainImageViews.size(); ++i)
	{
		const vk::ImageView attachments[] = { m_swapChainImageViews[i] };

		const vk::FramebufferCreateInfo framebufferInfo
		{
			.renderPass = m_renderPass,
			.attachmentCount = 1,
			.pAttachments = attachments,
			.width = m_swapChainExtent.width,
			.height = m_swapChainExtent.height,
			.layers = 1,
		};

		m_swapChainFramebuffers[i] = m_device.createFramebuffer(framebufferInfo, nullptr);
		if (!m_swapChainFramebuffers[i])
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void HelloTriangleApplication::cleanupSwapchain() const
{
	for (vk::Framebuffer framebuffer : m_swapChainFramebuffers)
	{
		vkDestroyFramebuffer(m_device, framebuffer, nullptr);
	}

	for (vk::ImageView imageView : m_swapChainImageViews)
	{
		vkDestroyImageView(m_device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
}

void HelloTriangleApplication::createCommandPool()
{
	const QueueFamilyIndices queueFamilyIndices = QueueFamilyUtils::findQueueFamilies(m_physicalDevice, m_surface);

	const vk::CommandPoolCreateInfo poolInfo
	{
		.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		.queueFamilyIndex = *queueFamilyIndices.get(QueueFamilyType::Graphics),
	};

	m_commandPool = m_device.createCommandPool(poolInfo, nullptr);
	if (!m_commandPool)
	{
		throw std::runtime_error("failed to create command pool!");
	}

	const vk::CommandPoolCreateInfo transferPoolInfo
	{
		.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		.queueFamilyIndex = *queueFamilyIndices.get(QueueFamilyType::Transfer),
	};

	m_transferCommandPool = m_device.createCommandPool(transferPoolInfo, nullptr);

	if (!m_transferCommandPool)
	{
		throw std::runtime_error("failed to create transfer command pool!");
	}
}

void HelloTriangleApplication::createVertexBuffer()
{
	const CreateDataBufferInfo info
	{
		.device = m_device,
		.physicalDevice = m_physicalDevice,
		.surface = m_surface,
		.usageType = vk::BufferUsageFlagBits::eVertexBuffer,
		.transferQueue = m_transferQueue,
		.transferCommandPool = m_transferCommandPool,
	};
	std::tie(m_vertexBuffer, m_vertexBufferMemory) = createDataBuffer(mesh.vertices, info);
}

void HelloTriangleApplication::createIndexBuffer()
{
	const CreateDataBufferInfo info
	{
		.device = m_device,
		.physicalDevice = m_physicalDevice,
		.surface = m_surface,
		.usageType = vk::BufferUsageFlagBits::eIndexBuffer,
		.transferQueue = m_transferQueue,
		.transferCommandPool = m_transferCommandPool,
	};
	std::tie(m_indexBuffer, m_indexBufferMemory) = createDataBuffer(mesh.indices, info);
}

void HelloTriangleApplication::createUniformBuffers()
{
	static constexpr vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

	m_uniformBuffers.resize(MaxFramesInFlight);
	m_uniformBuffersMemory.resize(MaxFramesInFlight);
	m_uniformBuffersMapped.resize(MaxFramesInFlight);

	const CreateBufferInfo bufferInfo
	{
		.device = m_device,
		.physicalDevice = m_physicalDevice,
		.size = bufferSize,
		.usage = vk::BufferUsageFlagBits::eUniformBuffer,
		.properties =  vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
	};

	auto bufferRange = std::views::zip(m_uniformBuffers, m_uniformBuffersMemory, m_uniformBuffersMapped);

	for (auto&& [buffer, memory, mapped] : bufferRange)
	{
		std::tie(buffer, memory) = createBuffer(bufferInfo);

		vkMapMemory(m_device, memory, 0, bufferSize, 0, &mapped);
	}
}

void HelloTriangleApplication::createCommandBuffers()
{
	const vk::CommandBufferAllocateInfo allocInfo
	{
		.commandPool = m_commandPool,
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = static_cast<uint32_t>(MaxFramesInFlight),
	};

	m_commandBuffers = m_device.allocateCommandBuffers(allocInfo);
	if (m_commandBuffers.empty())
	{
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

void HelloTriangleApplication::createSyncObjects()
{
	constexpr vk::FenceCreateInfo fenceInfo
	{
		.flags = vk::FenceCreateFlagBits::eSignaled
	};

	m_imageAvailableSemaphores.resize(MaxFramesInFlight);
	m_renderFinishedSemaphores.resize(MaxFramesInFlight);
	m_inFlightFences.resize(MaxFramesInFlight);

	for (auto&& [imageAvailable, renderFinished, fence] : std::views::zip(m_imageAvailableSemaphores, m_renderFinishedSemaphores, m_inFlightFences))
	{
		constexpr vk::SemaphoreCreateInfo semaphoreInfo;
		imageAvailable = m_device.createSemaphore(semaphoreInfo, nullptr);
		renderFinished = m_device.createSemaphore(semaphoreInfo, nullptr);
		fence = m_device.createFence(fenceInfo, nullptr);

		if (!imageAvailable || !renderFinished || !fence)
		{
			throw std::runtime_error("failed to create semaphores!");
		}
	}
}

void HelloTriangleApplication::createDescriptorPool()
{
	constexpr vk::DescriptorPoolSize poolSizes[]
	{
		{ vk::DescriptorType::eCombinedImageSampler, 1 },
	};

	const vk::DescriptorPoolCreateInfo poolInfo
	{
		.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
		.maxSets = 1,
		.poolSizeCount = static_cast<uint32_t>(IM_ARRAYSIZE(poolSizes)),
		.pPoolSizes = poolSizes,
	};

	m_descriptorPool = m_device.createDescriptorPool(poolInfo, nullptr);
}

void HelloTriangleApplication::initImguiHelper() const
{
	ImGui_ImplVulkan_InitInfo imguiInitInfo
	{
		.Instance = m_instance,
		.PhysicalDevice = m_physicalDevice,
		.Device = m_device,
		.QueueFamily = *QueueFamilyUtils::findQueueFamilies(m_physicalDevice, m_surface).get(QueueFamilyType::Graphics),
		.Queue = m_graphicsQueue,
		.DescriptorPool = m_descriptorPool,
		.RenderPass = m_renderPass,
		.MinImageCount = 2,
		.ImageCount = static_cast<uint32_t>(m_swapChainImageViews.size()),
		.MSAASamples = VK_SAMPLE_COUNT_1_BIT,
		.PipelineCache = m_pipelineCache,
		.Subpass = 0,
		.Allocator = nullptr,
		.CheckVkResultFn = nullptr,
	};
	imguiHelper.init(m_window, imguiInitInfo);
}

void HelloTriangleApplication::pickPhysicalDevice()
{
	const std::vector<vk::PhysicalDevice> devices = m_instance.enumeratePhysicalDevices();
	if (devices.empty())
		throw std::runtime_error("failed to find GPUs with Vulkan support!");

	auto isDeviceSuitable = [&](vk::PhysicalDevice device)
	{
		if (!QueueFamilyUtils::areAllIndicesSet(QueueFamilyUtils::findQueueFamilies(device, m_surface))
			|| !checkDeviceExtensionSupport(device))
			return false;

		const SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, m_surface);
		return !swapChainSupport.formats.empty()
			&& !swapChainSupport.presentModes.empty();
	};

	auto found = std::ranges::find_if(devices, isDeviceSuitable);

	if (found != devices.end())
	{
		m_physicalDevice = *found;		
		vk::PhysicalDeviceProperties properties = m_physicalDevice.getProperties();
		std::cout << "Using device: " << properties.deviceName << "\n";
	}
	else
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

bool HelloTriangleApplication::checkValidationLayerSupport()
{
	const std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

	auto isLayerAvailable = [&](const char* layerName) {
		auto matchesName = [&](const vk::LayerProperties& item) { return strcmp(layerName, item.layerName) == 0; };
		return std::ranges::any_of(availableLayers, matchesName); };

	return std::ranges::all_of(ValidationLayers, isLayerAvailable);
}

uint32_t findMemoryType(vk::PhysicalDevice physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties)
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

std::tuple<vk::Buffer, vk::DeviceMemory> createBuffer(const CreateBufferInfo& info)
{
	std::tuple<vk::Buffer, vk::DeviceMemory> result;
	auto& [buffer, memory] = result;

	const vk::BufferCreateInfo bufferInfo
	{
		.size = info.size,
		.usage = info.usage,
		.sharingMode = info.queueFamilyIndices.size() > 1u ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive,
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
		.memoryTypeIndex = findMemoryType(info.physicalDevice, memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent),
	};

	memory = info.device.allocateMemory(allocInfo, nullptr);
	if (!memory)
	{
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}

	info.device.bindBufferMemory(buffer, memory, 0);

	return result;
}

std::tuple<vk::Buffer, vk::DeviceMemory> createDataBuffer(const void* data, vk::DeviceSize size, const CreateDataBufferInfo& info)
{
	auto allFamilyIndices = QueueFamilyUtils::findQueueFamilies(info.physicalDevice, info.surface);
	auto familyIndicesToUse = { *allFamilyIndices.get(QueueFamilyType::Graphics), *allFamilyIndices.get(QueueFamilyType::Transfer) };

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

	void* mapData;
	vkMapMemory(info.device, stagingBufferMemory, 0, size, 0, &mapData);
	memcpy(mapData, data, static_cast<size_t>(size));
	vkUnmapMemory(info.device, stagingBufferMemory);

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

	copyBuffer(info.device, info.transferCommandPool, info.transferQueue, size, stagingBuffer, std::get<vk::Buffer>(result));
	vkDestroyBuffer(info.device, stagingBuffer, nullptr);
	vkFreeMemory(info.device, stagingBufferMemory, nullptr);

	return result;
}

void copyBuffer(vk::Device device, vk::CommandPool commandPool, vk::Queue queue, vk::DeviceSize size, vk::Buffer srcBuffer, vk::Buffer dstBuffer)
{
	const vk::CommandBufferAllocateInfo allocInfo
	{
		.commandPool = commandPool,
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = 1,
	};
	const vk::CommandBuffer commandBuffer = *device.allocateCommandBuffers(allocInfo).begin();

	constexpr vk::CommandBufferBeginInfo beginInfo
	{
		.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
	};
	commandBuffer.begin(beginInfo);

	const vk::BufferCopy copyRegion
	{
		.srcOffset = 0, // Optional
		.dstOffset = 0, // Optional
		.size = size,
	};
	commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);
	commandBuffer.end();

	const vk::SubmitInfo submitInfo
	{

		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer,
	};

	queue.submit(submitInfo, nullptr);
	queue.waitIdle();
	device.freeCommandBuffers(commandPool, 1, &commandBuffer);
}

void HelloTriangleApplication::recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex) const
{
	constexpr vk::CommandBufferBeginInfo beginInfo
	{
		.flags = {},
		.pInheritanceInfo = nullptr, // Optional
	};

	commandBuffer.begin(beginInfo);

	constexpr vk::ClearValue clearColor{ {0.0f, 0.0f, 0.0f, 1.0f} };

	const vk::RenderPassBeginInfo renderPassInfo
	{
		.renderPass = m_renderPass,
		.framebuffer = m_swapChainFramebuffers[imageIndex],
		.renderArea
		{
			.offset = {0, 0},
			.extent = m_swapChainExtent,
		},
		.clearValueCount = 1,
		.pClearValues = &clearColor,
	};
	commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_graphicsPipeline);

	const vk::Viewport viewport
	{
		.x = 0.0f,
		.y = 0.0f,
		.width = static_cast<float>(m_swapChainExtent.width),
		.height = static_cast<float>(m_swapChainExtent.height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};
	commandBuffer.setViewport(0, 1, &viewport);

	const vk::Rect2D scissor
	{
		.offset = { 0, 0 },
		.extent = m_swapChainExtent,
	};
	commandBuffer.setScissor(0, 1, &scissor);

	const vk::Buffer vertexBuffers[] = { m_vertexBuffer };
	constexpr vk::DeviceSize offsets[] = { 0 };
	commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);
	commandBuffer.bindIndexBuffer(m_indexBuffer, 0, vk::IndexType::eUint16);

	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(mesh.indices.size()), 1, 0, 0, 0);

	imguiHelper.renderFrame(commandBuffer);

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to record command buffer!");
	}
}

void HelloTriangleApplication::updateUniformBuffer(uint32_t currentImage, float deltaTime) const
{
	UniformBufferObject uniformBufferObject
	{
		.model = glm::rotate(glm::mat4(1.0f), deltaTime * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		.proj = glm::perspective(glm::radians(45.0f), m_swapChainExtent.width / static_cast<float>(m_swapChainExtent.height), 0.1f, 10.0f),
	};
	uniformBufferObject.proj[1][1] *= -1;
		
	memcpy(m_uniformBuffersMapped[currentImage], &uniformBufferObject, sizeof(uniformBufferObject));
}

void HelloTriangleApplication::drawFrame(float deltaTime)
{
	const vk::Fence fence = m_inFlightFences[currentFrame];
	const vk::Semaphore imageAvailableSemaphore = m_imageAvailableSemaphores[currentFrame];
	const vk::Semaphore renderFinishedSemaphore = m_renderFinishedSemaphores[currentFrame];
	const vk::CommandBuffer commandBuffer = m_commandBuffers[currentFrame];

	if (m_device.waitForFences(1, &fence, vk::False, UINT64_MAX) != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to wait for fences!");
	}

	imguiHelper.drawFrame();

	auto&& [imageResult, imageIndex] = m_device.acquireNextImageKHR(m_swapChain, UINT64_MAX, imageAvailableSemaphore,
	                                                           nullptr);

	if (imageResult == vk::Result::eErrorOutOfDateKHR)
	{
		recreateSwapchain();
		return;
	}
	else if (imageResult != vk::Result::eSuccess && imageResult != vk::Result::eSuboptimalKHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	if (m_device.resetFences(1, &fence) != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to reset fences!");
	}

	commandBuffer.reset();
	recordCommandBuffer(commandBuffer, imageIndex);

	updateUniformBuffer(currentFrame, deltaTime);

	const vk::Semaphore waitSemaphores[] = { imageAvailableSemaphore };
	const vk::Semaphore signalSemaphores[] = { renderFinishedSemaphore };
	static constexpr vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

	const vk::SubmitInfo submitInfo
	{
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = waitSemaphores,
		.pWaitDstStageMask = waitStages,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = signalSemaphores,
	};

	if (m_graphicsQueue.submit(1, &submitInfo, fence) != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	vk::SwapchainKHR swapChains[] = { m_swapChain };

	const vk::PresentInfoKHR presentInfo
	{

		.waitSemaphoreCount = 1,
		.pWaitSemaphores = signalSemaphores,
		.swapchainCount = 1,
		.pSwapchains = swapChains,
		.pImageIndices = &imageIndex,
		.pResults = nullptr, // Optional
	};

	if (const vk::Result result = m_presentQueue.presentKHR(presentInfo);
		result == vk::Result::eErrorOutOfDateKHR || result ==  vk::Result::eSuboptimalKHR || m_framebufferResized)
	{
		m_framebufferResized = false;
		recreateSwapchain();
		return;
	}
	else if (result != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	currentFrame = (currentFrame + 1) % MaxFramesInFlight;
}

std::vector<const char*> HelloTriangleApplication::getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (vk::EnableValidationLayers)
	{
		extensions.push_back(vk::EXTDebugUtilsExtensionName);
	}

	return extensions;
}

void HelloTriangleApplication::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto app = static_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
	app->m_framebufferResized = true;
}

void ImGuiHelper::init(GLFWwindow* window, ImGui_ImplVulkan_InitInfo& initInfo)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Init ImGui
	ImGui_ImplGlfw_InitForVulkan(window, true);
	ImGui_ImplVulkan_Init(&initInfo);
}

void ImGuiHelper::drawFrame()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (m_showDemoWindow)
		ImGui::ShowDemoWindow(&m_showDemoWindow);
}

void ImGuiHelper::renderFrame(vk::CommandBuffer commandBuffer)
{
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

void ImGuiHelper::shutdown()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

vk::VertexInputBindingDescription Vertex::getBindingDescription()
{
	constexpr vk::VertexInputBindingDescription bindingDescription
	{
		.binding = 0,
		.stride = sizeof(Vertex),
		.inputRate = vk::VertexInputRate::eVertex,
	};

	return bindingDescription;
}

std::array<vk::VertexInputAttributeDescription, 2> Vertex::getAttributeDescriptions()
{
	return
	{
		vk::VertexInputAttributeDescription
		{
			.location = 0,
			.binding = 0,
			.format = vk::Format::eR32G32Sfloat,
			.offset = offsetof(Vertex, pos),
		},
		vk::VertexInputAttributeDescription
		{
			.location = 1,
			.binding = 0,
			.format = vk::Format::eR32G32B32Sfloat,
			.offset = offsetof(Vertex, color),
		},
	};
}