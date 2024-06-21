module;

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

module Engine.Render.Application;

import Engine.AssetManager;
import Engine.Config;
import Engine.Core;
import Engine.Render.Core;

VulkanApplication::~VulkanApplication() noexcept
{
    if (!m_terminated)
    {
        shutdown();
    }
}

void VulkanApplication::init(ApplicationState& applicationState)
{
    m_windowSize = Config::getApplicationSettings().resolution;
    
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
        std::cout << "Loader/Runtime support detected for Vulkan " << VK_VERSION_MAJOR(apiVersion) << "." <<
            VK_VERSION_MINOR(apiVersion) << "\n";

        createInstance();

        RenderUtils::createDebugUtilsMessenger(m_instance, &m_debugMessenger, nullptr);
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapchain();
        createImageViews();
        createRenderPass();
        createDescriptorSetLayout();
        createGraphicsPipeline();
        createCommandPool();
        createDepthResources();
        createFramebuffers();
        createCommandBuffers();
        createSyncObjects();
        createDescriptorPool();
    }

    const ImGuiInitInfo imguiInfo
    {
        .instance = m_instance,
        .physicalDevice = m_physicalDevice,
        .device = m_device,
        .surface = m_surface,
        .queue = m_graphicsQueue,
        .renderPass = m_renderPass,
        .imageCount = m_swapChainImageViews.size(),
        .pipelineCache = m_pipelineCache
    };
    m_imguiHelper.init(m_window, imguiInfo);

    renderObjectManager.init(m_device, m_physicalDevice, m_surface, m_descriptorPool, m_descriptorSetLayout,
                       m_graphicsQueue, m_commandPool);
    applicationState.initialized = true;
    applicationState.application = this;
}

void VulkanApplication::update(float deltaTime)
{
    m_graphicsQueue.waitIdle(); // TODO: Optimization target. Explore VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT.
    glfwPollEvents();
    drawFrame(deltaTime);
}

void VulkanApplication::shutdown()
{
    m_terminated = true;
    m_device.waitIdle();

    renderObjectManager.shutdown();
    m_imguiHelper.shutdown();

    cleanupSwapchain();

    m_device.destroyDescriptorPool(m_descriptorPool);
    m_device.destroyDescriptorSetLayout(m_descriptorSetLayout);
    m_device.destroyCommandPool(m_commandPool);
    m_device.destroyCommandPool(m_transferCommandPool);
    m_device.destroyPipeline(m_graphicsPipeline);
    m_device.destroyPipelineLayout(m_pipelineLayout);
    m_device.destroyRenderPass(m_renderPass);

    for (size_t i = 0; i < MaxFramesInFlight; ++i)
    {
        m_device.destroySemaphore(m_imageAvailableSemaphores[i]);
        m_device.destroySemaphore(m_renderFinishedSemaphores[i]);
        m_device.destroyFence(m_inFlightFences[i]);
    }

    m_device.destroy();
    m_instance.destroySurfaceKHR(m_surface);

    if (vk::EnableValidationLayers)
    {
        RenderUtils::destroyDebugUtilsMessenger(m_instance, m_debugMessenger, nullptr);
    }

    m_instance.destroy();
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

bool VulkanApplication::shouldWindowClose() const
{
    return glfwWindowShouldClose(m_window);
}

void VulkanApplication::createInstance()
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

    const auto debugCreateInfo = RenderUtils::newDebugUtilsMessengerCreateInfo();

    if (vk::EnableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(RenderUtils::ValidationLayers.size());
        createInfo.ppEnabledLayerNames = RenderUtils::ValidationLayers.data();
        createInfo.pNext = &debugCreateInfo;
    }

    m_instance = vk::createInstance(createInfo);

    vk::defaultDispatchLoaderDynamic.init(m_instance);
}

void VulkanApplication::createSurface()
{
    if (glfw::createWindowSurface(m_instance, m_window, nullptr, &m_surface) != vk::Result::eSuccess)
        throw std::runtime_error("failed to create window surface!");
}

void VulkanApplication::createLogicalDevice()
{
    const QueueFamilyIndices indices = QueueFamilyUtils::findQueueFamilies(m_physicalDevice, m_surface);

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;

    queueCreateInfos.reserve(indices.size());

    auto indexRange = indices | std::views::transform([&](auto&& index) { return *index; });
    std::set<std::optional<uint32_t>> uniqueQueueFamilies(indexRange.begin(), indexRange.end());

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

    const vk::PhysicalDeviceFeatures deviceFeatures
    {
        .samplerAnisotropy = vk::True,
    };

    constexpr vk::PhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures
    {
        .descriptorBindingUniformBufferUpdateAfterBind = true,
        .descriptorBindingSampledImageUpdateAfterBind = true,
        .descriptorBindingStorageImageUpdateAfterBind = true,
        .descriptorBindingStorageBufferUpdateAfterBind = true,
        .descriptorBindingUniformTexelBufferUpdateAfterBind = true,
        .descriptorBindingStorageTexelBufferUpdateAfterBind = true,
    };

    const vk::DeviceCreateInfo createInfo
    {
        .pNext = &descriptorIndexingFeatures,
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledLayerCount = [&]
        {
            if constexpr (vk::EnableValidationLayers)
                return static_cast<uint32_t>(RenderUtils::ValidationLayers.
                    size());
            else return 0;
        }(),
        .ppEnabledLayerNames = [&]
        {
            if constexpr (vk::EnableValidationLayers) return RenderUtils::ValidationLayers.data();
            else return nullptr;
        }(),
        .enabledExtensionCount = static_cast<uint32_t>(RenderUtils::DeviceExtensions.size()),
        .ppEnabledExtensionNames = RenderUtils::DeviceExtensions.data(),
        .pEnabledFeatures = &deviceFeatures,
    };

    m_device = m_physicalDevice.createDevice(createInfo);
    if (!m_device)
        throw std::runtime_error("failed to create logical device!");

    m_graphicsQueue = m_device.getQueue(*indices.get(QueueFamilyType::Graphics), 0);
    m_presentQueue = m_device.getQueue(*indices.get(QueueFamilyType::Present), 0);
    m_transferQueue = m_device.getQueue(*indices.get(QueueFamilyType::Transfer), 0);
}

void VulkanApplication::recreateSwapchain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_window, &width, &height);
    while (width == 0 || height == 0) // NOLINT(bugprone-infinite-loop)
    {
        glfwGetFramebufferSize(m_window, &width, &height);
        glfwWaitEvents();
    }

    m_device.waitIdle();

    cleanupSwapchain();

    createSwapchain();
    createImageViews();
    createDepthResources();
    createFramebuffers();
}

void VulkanApplication::createSwapchain()
{
    const RenderUtils::SwapChainSupportDetails swapChainSupport = RenderUtils::querySwapChainSupport(
        m_physicalDevice, m_surface);

    const vk::SurfaceFormatKHR surfaceFormat = RenderUtils::chooseSwapSurfaceFormat(swapChainSupport.formats);
    const vk::PresentModeKHR presentMode = RenderUtils::chooseSwapPresentMode(swapChainSupport.presentModes);
    const vk::Extent2D extent = RenderUtils::chooseSwapExtent(swapChainSupport.capabilities, m_window);

    uint32_t minImageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && minImageCount > swapChainSupport.capabilities.maxImageCount)
    {
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
    m_swapchainExtent = extent;
}

void VulkanApplication::createImageViews()
{
    m_swapChainImageViews.resize(m_swapChainImages.size());

    for (size_t i = 0; i < m_swapChainImages.size(); ++i)
    {
        m_swapChainImageViews[i] =
            RenderUtils::createImageView(m_device, m_swapChainImages[i], m_swapChainImageFormat);
    }
}

void VulkanApplication::createRenderPass()
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

    const vk::AttachmentDescription depthAttachment
    {
        .format = RenderUtils::findDepthFormat(m_physicalDevice),
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eDontCare,
        .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
        .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal
    };

    const vk::AttachmentReference depthAttachmentRef
    {
        .attachment = 1,
        .layout = vk::ImageLayout::eDepthStencilAttachmentOptimal
    };

    const vk::SubpassDescription subpass
    {
        .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
        .pDepthStencilAttachment = &depthAttachmentRef
    };

    constexpr vk::SubpassDependency dependency
    {
        .srcSubpass = vk::SubpassExternal,
        .dstSubpass = 0,
        .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput |
        vk::PipelineStageFlagBits::eEarlyFragmentTests,
        .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput |
        vk::PipelineStageFlagBits::eEarlyFragmentTests,
        .srcAccessMask = {},
        .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
    };

    const std::array attachments = {colorAttachment, depthAttachment};

    const vk::RenderPassCreateInfo renderPassInfo
    {
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
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

void VulkanApplication::createDescriptorSetLayout()
{
    constexpr vk::DescriptorSetLayoutBinding layoutBinding
    {
        .binding = 0,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eVertex,
        .pImmutableSamplers = nullptr, // Optional
    };

    constexpr vk::DescriptorSetLayoutBinding samplerLayoutBinding
    {
        .binding = 1,
        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eFragment,
        .pImmutableSamplers = nullptr, // Optional
    };

    constexpr std::array bindings{layoutBinding, samplerLayoutBinding};
    const vk::DescriptorSetLayoutCreateInfo layoutInfo
    {
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data(),
    };

    m_descriptorSetLayout = m_device.createDescriptorSetLayout(layoutInfo, nullptr);
    if (!m_descriptorSetLayout)
    {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void VulkanApplication::createGraphicsPipeline()
{
    auto vertShaderCode = RenderUtils::readFile(AssetManager::getExecutableRoot() + "Shaders/vert.spv");
    auto fragShaderCode = RenderUtils::readFile(AssetManager::getExecutableRoot() + "Shaders/frag.spv");

    vk::ShaderModule vertShaderModule = RenderUtils::createShaderModule(vertShaderCode, m_device);
    vk::ShaderModule fragShaderModule = RenderUtils::createShaderModule(fragShaderCode, m_device);

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

    const vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    const std::vector dynamicStates
    {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    const vk::PipelineDynamicStateCreateInfo dynamicStateInfo
    {
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data(),
    };

    constexpr vk::VertexInputBindingDescription bindingDescription
    {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = vk::VertexInputRate::eVertex,
    };

    constexpr std::array attributeDescriptions
    {
        vk::VertexInputAttributeDescription
        {
            .location = 0,
            .binding = 0,
            .format = vk::Format::eR32G32B32Sfloat,
            .offset = offsetof(Vertex, pos),
        },
        vk::VertexInputAttributeDescription
        {
            .location = 1,
            .binding = 0,
            .format = vk::Format::eR32G32Sfloat,
            .offset = offsetof(Vertex, texCoordinates),
        },
    };

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
        .width = static_cast<float>(m_swapchainExtent.width),
        .height = static_cast<float>(m_swapchainExtent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    const vk::Rect2D scissor
    {
        .offset = {0, 0},
        .extent = m_swapchainExtent,
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
        .frontFace = vk::FrontFace::eCounterClockwise,
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
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
    };

    const vk::PipelineColorBlendStateCreateInfo colorBlendingInfo
    {
        .logicOpEnable = vk::False,
        .logicOp = vk::LogicOp::eCopy, // Optional
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants = std::array{0.f, 0.f, 0.f, 0.f}
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

    const vk::PipelineDepthStencilStateCreateInfo depthStencil
    {
        .depthTestEnable = vk::True,
        .depthWriteEnable = vk::True,
        .depthCompareOp = vk::CompareOp::eLess,
        .depthBoundsTestEnable = vk::False,
        .stencilTestEnable = vk::False,
        .front = {}, // Optional
        .back = {}, // Optional
        .minDepthBounds = 0.0f, // Optional
        .maxDepthBounds = 1.0f, // Optional
    };

    const vk::GraphicsPipelineCreateInfo pipelineInfo
    {
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssemblyInfo,
        .pViewportState = &viewportStateInfo,
        .pRasterizationState = &rasterizerInfo,
        .pMultisampleState = &multisamplingInfo,
        .pDepthStencilState = &depthStencil,
        .pColorBlendState = &colorBlendingInfo,
        .pDynamicState = &dynamicStateInfo,
        .layout = m_pipelineLayout,
        .renderPass = m_renderPass,
        .subpass = 0,
        .basePipelineHandle = nullptr,
        .basePipelineIndex = -1,
    };

    auto&& [result, pipeline] = m_device.createGraphicsPipeline(m_pipelineCache, pipelineInfo);

    m_graphicsPipeline = pipeline;

    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    m_device.destroyShaderModule(fragShaderModule, nullptr);
    m_device.destroyShaderModule(vertShaderModule, nullptr);
}

void VulkanApplication::createFramebuffers()
{
    m_swapChainFramebuffers.resize(m_swapChainImageViews.size());

    for (size_t i = 0; i < m_swapChainImageViews.size(); ++i)
    {
        const std::array attachments{m_swapChainImageViews[i], m_depthImageView};

        const vk::FramebufferCreateInfo framebufferInfo
        {
            .renderPass = m_renderPass,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .width = m_swapchainExtent.width,
            .height = m_swapchainExtent.height,
            .layers = 1
        };

        m_swapChainFramebuffers[i] = m_device.createFramebuffer(framebufferInfo, nullptr);
        if (!m_swapChainFramebuffers[i])
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void VulkanApplication::cleanupSwapchain() const
{
    for (vk::Framebuffer framebuffer : m_swapChainFramebuffers)
        m_device.destroyFramebuffer(framebuffer);

    for (vk::ImageView imageView : m_swapChainImageViews)
        m_device.destroyImageView(imageView);

    m_device.destroySwapchainKHR(m_swapChain);

    m_device.destroyImageView(m_depthImageView);
    m_device.destroyImage(m_depthImage);
    m_device.freeMemory(m_depthImageMemory);
}

void VulkanApplication::createCommandPool()
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

void VulkanApplication::createCommandBuffers()
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

void VulkanApplication::createSyncObjects()
{
    constexpr vk::FenceCreateInfo fenceInfo
    {
        .flags = vk::FenceCreateFlagBits::eSignaled
    };

    m_imageAvailableSemaphores.resize(MaxFramesInFlight);
    m_renderFinishedSemaphores.resize(MaxFramesInFlight);
    m_inFlightFences.resize(MaxFramesInFlight);

    for (auto&& [imageAvailable, renderFinished, fence] : std::views::zip(
             m_imageAvailableSemaphores, m_renderFinishedSemaphores, m_inFlightFences))
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

void VulkanApplication::createDescriptorPool()
{
    constexpr uint32_t count = MaxFramesInFlight;
    constexpr std::array poolSizes
    {
        vk::DescriptorPoolSize{vk::DescriptorType::eUniformBuffer, count},
        vk::DescriptorPoolSize{vk::DescriptorType::eCombinedImageSampler, count},
    };

    const vk::DescriptorPoolCreateInfo poolInfo
    {
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = 1000, // TODO figure this out
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data(),
    };

    m_descriptorPool = m_device.createDescriptorPool(poolInfo, nullptr);
}

void VulkanApplication::pickPhysicalDevice()
{
    const std::vector<vk::PhysicalDevice> devices = m_instance.enumeratePhysicalDevices();
    if (devices.empty())
        throw std::runtime_error("failed to find GPUs with Vulkan support!");

    auto isDeviceSuitable = [&](vk::PhysicalDevice device)
    {
        if (!QueueFamilyUtils::areAllIndicesSet(QueueFamilyUtils::findQueueFamilies(device, m_surface))
            || !RenderUtils::checkDeviceExtensionSupport(device))
            return false;

        const RenderUtils::SwapChainSupportDetails swapChainSupport = RenderUtils::querySwapChainSupport(
            device, m_surface);
        if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty())
            return false;

        const vk::PhysicalDeviceFeatures features = device.getFeatures();
        if (!features.samplerAnisotropy)
            return false;
        return true;
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

bool VulkanApplication::checkValidationLayerSupport()
{
    const std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

    auto isLayerAvailable = [&](const char* layerName)
    {
        auto matchesName = [&](const vk::LayerProperties& item) { return strcmp(layerName, item.layerName) == 0; };
        return std::ranges::any_of(availableLayers, matchesName);
    };

    return std::ranges::all_of(RenderUtils::ValidationLayers, isLayerAvailable);
}

void VulkanApplication::drawFrame(float deltaTime)
{
    const vk::Fence fence = m_inFlightFences[m_currentFrame];
    const vk::Semaphore imageAvailableSemaphore = m_imageAvailableSemaphores[m_currentFrame];
    const vk::Semaphore renderFinishedSemaphore = m_renderFinishedSemaphores[m_currentFrame];
    const vk::CommandBuffer commandBuffer = m_commandBuffers[m_currentFrame];

    if (m_device.waitForFences(1, &fence, vk::False, UINT64_MAX) != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to wait for fences!");
    }

    const auto imageResult = m_device.acquireNextImageKHR(m_swapChain, UINT64_MAX, imageAvailableSemaphore,
                                                          nullptr);

    if (imageResult.result == vk::Result::eErrorOutOfDateKHR)
    {
        recreateSwapchain();
        return;
    }
    else if (imageResult.result != vk::Result::eSuccess && imageResult.result != vk::Result::eSuboptimalKHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    if (m_device.resetFences(1, &fence) != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to reset fences!");
    }

    m_imguiHelper.drawFrame();

    commandBuffer.reset();

    constexpr vk::CommandBufferBeginInfo beginInfo
    {
        .flags = {},
        .pInheritanceInfo = nullptr, // Optional
    };

    commandBuffer.begin(beginInfo);

    constexpr std::array clearColor
    {
        vk::ClearValue{vk::ClearColorValue{0.0f, 0.0f, 0.0f, 1.0f}},
        vk::ClearValue{vk::ClearDepthStencilValue{1.0f, 0}},
    };

    const vk::RenderPassBeginInfo renderPassInfo
    {
        .renderPass = m_renderPass,
        .framebuffer = m_swapChainFramebuffers[imageResult.value],
        .renderArea
        {
            .offset = {0, 0},
            .extent = m_swapchainExtent,
        },
        .clearValueCount = static_cast<uint32_t>(clearColor.size()),
        .pClearValues = clearColor.data(),
    };

    const vk::Viewport viewport
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(m_swapchainExtent.width),
        .height = static_cast<float>(m_swapchainExtent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    commandBuffer.setViewport(0, 1, &viewport);

    const vk::Rect2D scissor
    {
        .offset = {0, 0},
        .extent = m_swapchainExtent,
    };
    commandBuffer.setScissor(0, 1, &scissor);

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    renderObjectManager.renderFrame(commandBuffer, m_graphicsPipeline, m_pipelineLayout, m_swapchainExtent, deltaTime,
                              m_currentFrame);

    m_imguiHelper.renderFrame(commandBuffer);

    commandBuffer.endRenderPass();

    commandBuffer.end();

    const vk::Semaphore waitSemaphores[] = {imageAvailableSemaphore};
    const vk::Semaphore signalSemaphores[] = {renderFinishedSemaphore};
    static constexpr vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

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

    vk::SwapchainKHR swapChains[] = {m_swapChain};

    const vk::PresentInfoKHR presentInfo
    {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
        .swapchainCount = 1,
        .pSwapchains = swapChains,
        .pImageIndices = &imageResult.value,
        .pResults = nullptr, // Optional
    };

    bool shouldRecreateSwapchain;
    try
    {
        const vk::Result result = m_presentQueue.presentKHR(presentInfo);
        shouldRecreateSwapchain = result == vk::Result::eSuboptimalKHR || m_framebufferResized;
    }
    catch (const vk::OutOfDateKHRError&)
    {
        shouldRecreateSwapchain = true;
    }

    if (shouldRecreateSwapchain)
    {
        m_framebufferResized = false;
        recreateSwapchain();
        return;
    }
    m_currentFrame = (m_currentFrame + 1) % MaxFramesInFlight;
}

std::vector<const char*> VulkanApplication::getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (vk::EnableValidationLayers)
    {
        extensions.push_back(vk::EXTDebugUtilsExtensionName);
    }

    return extensions;
}

void VulkanApplication::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto app = static_cast<VulkanApplication*>(glfwGetWindowUserPointer(window));
    app->m_framebufferResized = true;
}

void ImGuiHelper::init(GLFWwindow* window, const ImGuiInitInfo& initInfo)
{
    if constexpr (!enabled)
        return;

    m_device = initInfo.device;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Init ImGui
    ImGui_ImplGlfw_InitForVulkan(window, true);

    constexpr std::array imguiPoolSizes
    {
        vk::DescriptorPoolSize{vk::DescriptorType::eCombinedImageSampler, 1},
    };

    const vk::DescriptorPoolCreateInfo imguiPoolInfo
    {
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = static_cast<uint32_t>(MaxFramesInFlight),
        .poolSizeCount = static_cast<uint32_t>(imguiPoolSizes.size()),
        .pPoolSizes = imguiPoolSizes.data(),
    };

    m_descriptorPool = initInfo.device.createDescriptorPool(imguiPoolInfo, nullptr);
    if (!m_descriptorPool)
        throw std::runtime_error("failed to create descriptor pool!");

    ImGui_ImplVulkan_InitInfo imguiInitInfo
    {
        .Instance = initInfo.instance,
        .PhysicalDevice = initInfo.physicalDevice,
        .Device = initInfo.device,
        .QueueFamily = *QueueFamilyUtils::findQueueFamilies(initInfo.physicalDevice, initInfo.surface).get(
            QueueFamilyType::Graphics),
        .Queue = initInfo.queue,
        .DescriptorPool = m_descriptorPool,
        .RenderPass = initInfo.renderPass,
        .MinImageCount = 2,
        .ImageCount = static_cast<uint32_t>(initInfo.imageCount),
        .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
        .PipelineCache = initInfo.pipelineCache,
        .Subpass = 0,
        .Allocator = nullptr,
        .CheckVkResultFn = nullptr,
    };
    ImGui_ImplVulkan_Init(&imguiInitInfo);
}

void ImGuiHelper::drawFrame()
{
    if constexpr (!enabled)
        return;

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (m_showDemoWindow)
        ImGui::ShowDemoWindow(&m_showDemoWindow);
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void ImGuiHelper::renderFrame(vk::CommandBuffer commandBuffer)
{
    if constexpr (!enabled)
        return;

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

void ImGuiHelper::shutdown()
{
    if constexpr (!enabled)
        return;

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    m_device.destroy(m_descriptorPool);
    m_descriptorPool = nullptr;
}

void VulkanApplication::createDepthResources()
{
    const vk::Format depthFormat = RenderUtils::findDepthFormat(m_physicalDevice);
    std::tie(m_depthImage, m_depthImageMemory) = RenderUtils::createImage
    (
        m_device,
        m_physicalDevice,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        m_swapchainExtent,
        depthFormat,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eDepthStencilAttachment
    );

    m_depthImageView = RenderUtils::createImageView(m_device, m_depthImage, depthFormat,
                                                    vk::ImageAspectFlagBits::eDepth);

    RenderUtils::transitionImageLayout(m_device, m_graphicsQueue, m_commandPool, m_depthImage, depthFormat,
                                       vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
}