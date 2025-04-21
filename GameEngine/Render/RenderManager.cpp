module Render.RenderManager;
import Core;
import Guid;
import Render.Model;
import Render.Pipeline.Line;
import Render.Pipeline.MeshWithTexture;
import Render.QueueFamily;
import Render.TextureLoading;
import Render.Utils;
import Wrapper.Vulkan;
import Wrapper.Windows;

std::mutex updateLockMutex;
std::atomic updatesBlocked{false};

[[nodiscard]]
vk::PipelineLayout createPipelineLayout(vk::Device device, vk::DescriptorSetLayout descriptorSetLayout)
{
    const vk::PipelineLayoutCreateInfo pipelineLayoutInfo
    {
        .setLayoutCount = 1,
        .pSetLayouts = &descriptorSetLayout,
        .pushConstantRangeCount = 0, // Optional
        .pPushConstantRanges = nullptr, // Optional
    };

    const vk::PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutInfo, nullptr);
    if (!pipelineLayout)
    {
        fatalError("failed to create pipeline layout!");
    }
    return pipelineLayout;
}

[[nodiscard]]
vk::RenderPass createRenderPass(vk::Device device, vk::PhysicalDevice physicalDevice, vk::Format swapChainImageFormat)
{
    const vk::AttachmentDescription colorAttachment
    {
        .format = swapChainImageFormat,
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
        .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout = vk::ImageLayout::ePresentSrcKHR,
    };

    static constexpr vk::AttachmentReference colorAttachmentRef
    {
        .attachment = 0,
        .layout = vk::ImageLayout::eColorAttachmentOptimal,
    };

    const vk::AttachmentDescription depthAttachment
    {
        .format = RenderUtils::findDepthFormat(physicalDevice),
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eDontCare,
        .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
        .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal
    };

    static constexpr vk::AttachmentReference depthAttachmentRef
    {
        .attachment = 1,
        .layout = vk::ImageLayout::eDepthStencilAttachmentOptimal
    };

    static constexpr vk::SubpassDescription subpass
    {
        .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
        .pDepthStencilAttachment = &depthAttachmentRef
    };

    static constexpr vk::SubpassDependency dependency
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
        .attachmentCount = static_cast<UInt32>(attachments.size()),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency
    };

    const vk::RenderPass renderPass = device.createRenderPass(renderPassInfo, nullptr);
    if (!renderPass)
    {
        fatalError("failed to create render pass!");
    }
    return renderPass;
}

[[nodiscard]]
vk::DescriptorSetLayout createDescriptorSetLayout(vk::Device device)
{
    static constexpr vk::DescriptorSetLayoutBinding layoutBinding
    {
        .binding = 0,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eVertex,
        .pImmutableSamplers = nullptr, // Optional
    };

    static constexpr vk::DescriptorSetLayoutBinding samplerLayoutBinding
    {
        .binding = 1,
        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eFragment,
        .pImmutableSamplers = nullptr, // Optional
    };

    static constexpr std::array bindings{layoutBinding, samplerLayoutBinding};
    static constexpr vk::DescriptorSetLayoutCreateInfo layoutInfo
    {
        .bindingCount = static_cast<UInt32>(bindings.size()),
        .pBindings = bindings.data(),
    };

    const vk::DescriptorSetLayout descriptorSetLayout = device.createDescriptorSetLayout(layoutInfo, nullptr);
    if (!descriptorSetLayout)
    {
        fatalError("failed to create descriptor set layout!");
    }
    return descriptorSetLayout;
}

[[nodiscard]]
vk::DescriptorPool createDescriptorPool(vk::Device device)
{
    static constexpr UInt32 count = MaxFramesInFlight;
    static constexpr std::array poolSizes
    {
        vk::DescriptorPoolSize{vk::DescriptorType::eUniformBuffer, count},
        vk::DescriptorPoolSize{vk::DescriptorType::eCombinedImageSampler, count},
    };

    static constexpr vk::DescriptorPoolCreateInfo poolInfo
    {
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = 1000, // TODO figure this out
        .poolSizeCount = static_cast<UInt32>(poolSizes.size()),
        .pPoolSizes = poolSizes.data(),
    };

    return device.createDescriptorPool(poolInfo, nullptr);
}

RenderManager::~RenderManager() noexcept
{
    if (!m_terminated)
    {
        shutdown();
    }
}

void RenderManager::init(GLFWwindow* window)
{
    // Init window
    check(!m_initialised, "[RenderManager] Tried to initialise more than once!");
    check(!m_window, "[RenderManager] Tried to create a new window without having deleted the current one!");
    check(window, "[RenderManager] Can't initialise without a window!");

    m_window = window;

    // Init Vulkan
    {
        vk::detail::defaultDispatchLoaderDynamic.init();

        // Determine what API version is available
        const UInt32 apiVersion = vk::enumerateInstanceVersion();
        std::cout << "Loader/Runtime support detected for Vulkan " << vk::apiVersionMajor(apiVersion) << "." <<
            vk::apiVersionMinor(apiVersion) << "\n";

        createInstance();

        RenderUtils::createDebugUtilsMessenger(m_instance, &m_debugMessenger, nullptr);
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapchain();
        createImageViews();
        m_descriptorPool = createDescriptorPool(m_device);
        m_descriptorSetLayout = createDescriptorSetLayout(m_device);
        m_pipelineLayout = createPipelineLayout(m_device, m_descriptorSetLayout);

        m_graphicsPipeline = createGraphicsPipeline(m_device, m_pipelineCache, m_pipelineLayout, m_swapchainExtent);
        m_linePipeline = createLinePipeline(m_device, m_pipelineCache, m_pipelineLayout, m_swapchainExtent);

        createCommandPool();
        createDepthResources();
        createCommandBuffers();
        createSyncObjects();
    }

    const ImGuiInitInfo imguiInfo
    {
        .instance = m_instance,
        .physicalDevice = m_physicalDevice,
        .device = m_device,
        .surface = m_surface,
        .queue = m_graphicsQueue,
        .imageCount = m_swapChainImageViews.size(),
        .pipelineCache = m_pipelineCache
    };
    m_imguiHelper.init(m_window, imguiInfo);

    m_renderObjectManager.init
    (
        m_device,
        m_physicalDevice,
        m_surface,
        m_descriptorPool,
        m_descriptorSetLayout,
        m_graphicsQueue,
        m_commandPool
    );

    m_initialised = true;
}

void RenderManager::update()
{
    if (updatesBlocked.load())
        return;

    std::lock_guard lock{updateLockMutex};

    m_graphicsQueue.waitIdle(); // TODO: Optimization target. Explore VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT.

    auto cmd = m_commands.tryPop();
    while (cmd.has_value())
    {
        cmd->get()->process();
        cmd = m_commands.tryPop();
    }

    m_deltaTime.update();
    drawFrame();
}

void RenderManager::shutdown()
{
    std::lock_guard lock{updateLockMutex};

    m_terminated = true;
    m_device.waitIdle();

    m_renderObjectManager.clear();
    m_imguiHelper.shutdown();

    cleanupSwapchain();

    m_device.destroyDescriptorPool(m_descriptorPool);
    m_device.destroyDescriptorSetLayout(m_descriptorSetLayout);
    m_device.destroyCommandPool(m_commandPool);
    m_device.destroyCommandPool(m_transferCommandPool);
    m_device.destroyPipeline(m_graphicsPipeline);
    m_device.destroyPipeline(m_linePipeline);
    m_device.destroyPipelineLayout(m_pipelineLayout);

    for (size_t i = 0; i < MaxFramesInFlight; ++i)
    {
        m_device.destroySemaphore(m_imageAvailableSemaphores[i]);
        m_device.destroySemaphore(m_renderFinishedSemaphores[i]);
        m_device.destroyFence(m_inFlightFences[i]);
    }

    m_device.destroy();
    m_instance.destroySurfaceKHR(m_surface);

    if constexpr (vk::EnableValidationLayers)
    {
        RenderUtils::destroyDebugUtilsMessenger(m_instance, m_debugMessenger, nullptr);
    }

    m_instance.destroy();
}

void RenderManager::clear()
{
    std::lock_guard lock{updateLockMutex};

    if (!m_initialised)
        return;

    updatesBlocked = true;

    m_device.waitIdle();
    m_renderObjectManager.clear();
    updatesBlocked = false;
}

void RenderManager::setCamera(const Camera& camera)
{
    m_renderObjectManager.setCamera(camera);
}

void RenderManager::addDebugWidget(std::unique_ptr<IDebugWidget> widget)
{
    m_imguiHelper.addWidget(std::move(widget));
}

void RenderManager::createInstance()
{
    if constexpr (vk::EnableValidationLayers)
        check(checkValidationLayerSupport(), "validation layers requested, but not available!");

    constexpr vk::ApplicationInfo appInfo
    {
        .pApplicationName = "Mashi Engine",
        .applicationVersion = vk::makeApiVersion(0, 1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = vk::makeApiVersion(0, 1, 0, 0),
        .apiVersion = vk::ApiVersion13
    };

    auto extensions = getRequiredExtensions();

    vk::InstanceCreateInfo createInfo
    {
        .pNext = nullptr,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = 0,
        .enabledExtensionCount = static_cast<UInt32>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };

    const auto debugCreateInfo = RenderUtils::newDebugUtilsMessengerCreateInfo();

    if constexpr (vk::EnableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<UInt32>(RenderUtils::ValidationLayers.size());
        createInfo.ppEnabledLayerNames = RenderUtils::ValidationLayers.data();
        createInfo.pNext = &debugCreateInfo;
    }

    m_instance = vk::createInstance(createInfo);

    vk::detail::defaultDispatchLoaderDynamic.init(m_instance);
}

void RenderManager::createSurface()
{
    check(glfw::createWindowSurface(m_instance, m_window, nullptr, &m_surface) == vk::Result::eSuccess, "Failed to create window surface!");
}

void RenderManager::createLogicalDevice()
{
    const QueueFamilyIndices indices = QueueFamilyUtils::findQueueFamilies(m_physicalDevice, m_surface);

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;

    queueCreateInfos.reserve(indices.size());

    auto indexRange = indices | std::views::transform([&](auto&& index) { return *index; });
    std::set<std::optional<UInt32>> uniqueQueueFamilies(indexRange.begin(), indexRange.end());

    for (std::optional<UInt32> queueFamily : uniqueQueueFamilies)
    {
        check(queueFamily.has_value(), "failed to find a unique queue family!");

        queueCreateInfos.emplace_back(vk::DeviceQueueCreateInfo
            {
                .queueFamilyIndex = *queueFamily,
                .queueCount = 1,
                .pQueuePriorities = &queuePriority,
            });
    }

    constexpr vk::PhysicalDeviceFeatures deviceFeatures
    {
        .fillModeNonSolid = vk::True,
        .samplerAnisotropy = vk::True,
    };

    constexpr vk::PhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures
    {
        .dynamicRendering = vk::True
    };

    const vk::DeviceCreateInfo createInfo
    {
        .pNext = &dynamicRenderingFeatures,
        .queueCreateInfoCount = static_cast<UInt32>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledLayerCount = [&]
        {
            if constexpr (vk::EnableValidationLayers)
                return static_cast<UInt32>(RenderUtils::ValidationLayers.size());
            else return 0;
        }(),
        .ppEnabledLayerNames = [&]
        {
            if constexpr (vk::EnableValidationLayers) return RenderUtils::ValidationLayers.data();
            else return nullptr;
        }(),
        .enabledExtensionCount = static_cast<UInt32>(RenderUtils::DeviceExtensions.size()),
        .ppEnabledExtensionNames = RenderUtils::DeviceExtensions.data(),
        .pEnabledFeatures = &deviceFeatures,
    };

    m_device = m_physicalDevice.createDevice(createInfo);
    check(m_device, "failed to create logical device!");

    m_graphicsQueue = m_device.getQueue(*indices.get(QueueFamilyType::Graphics), 0);
    m_presentQueue = m_device.getQueue(*indices.get(QueueFamilyType::Present), 0);
    m_transferQueue = m_device.getQueue(*indices.get(QueueFamilyType::Transfer), 0);
}

void RenderManager::recreateSwapchain()
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
}

void RenderManager::createSwapchain()
{
    const RenderUtils::SwapChainSupportDetails swapChainSupport = RenderUtils::querySwapChainSupport(m_physicalDevice, m_surface);

    const vk::SurfaceFormatKHR surfaceFormat = RenderUtils::chooseSwapSurfaceFormat(swapChainSupport.formats);
    const vk::PresentModeKHR presentMode = RenderUtils::chooseSwapPresentMode(swapChainSupport.presentModes);
    const vk::Extent2D extent = RenderUtils::chooseSwapExtent(swapChainSupport.capabilities, m_window);

    UInt32 minImageCount = swapChainSupport.capabilities.minImageCount + 1;

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
    auto indicesRange = indices | std::views::transform([](std::optional<UInt32> value) { return *value; });
    const std::vector<UInt32> queueFamilyIndices(indicesRange.begin(), indicesRange.end());

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
        fatalError("failed to create swap chain!");
    }

    m_swapChainImages = m_device.getSwapchainImagesKHR(m_swapChain);
    m_swapChainImageFormat = surfaceFormat.format;
    m_swapchainExtent = extent;
}

void RenderManager::createImageViews()
{
    m_swapChainImageViews.resize(m_swapChainImages.size());

    for (size_t i = 0; i < m_swapChainImages.size(); ++i)
    {
        m_swapChainImageViews[i] = RenderUtils::createImageView(m_device, m_swapChainImages[i], m_swapChainImageFormat);
    }
}

void RenderManager::cleanupSwapchain() const
{
    for (vk::ImageView imageView : m_swapChainImageViews)
        m_device.destroyImageView(imageView);

    m_device.destroySwapchainKHR(m_swapChain);

    m_device.destroyImageView(m_depthImageView);
    m_device.destroyImage(m_depthImage);
    m_device.freeMemory(m_depthImageMemory);
}

void RenderManager::createCommandPool()
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
        fatalError("failed to create command pool!");
    }

    const vk::CommandPoolCreateInfo transferPoolInfo
    {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = *queueFamilyIndices.get(QueueFamilyType::Transfer),
    };

    m_transferCommandPool = m_device.createCommandPool(transferPoolInfo, nullptr);

    if (!m_transferCommandPool)
    {
        fatalError("failed to create transfer command pool!");
    }
}

void RenderManager::createCommandBuffers()
{
    const vk::CommandBufferAllocateInfo allocInfo
    {
        .commandPool = m_commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = static_cast<UInt32>(MaxFramesInFlight),
    };

    m_commandBuffers = m_device.allocateCommandBuffers(allocInfo);
    if (m_commandBuffers.empty())
    {
        fatalError("failed to allocate command buffers!");
    }
}

void RenderManager::createSyncObjects()
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
            fatalError("failed to create semaphores!");
        }
    }
}

void RenderManager::pickPhysicalDevice()
{
    const std::vector<vk::PhysicalDevice> devices = m_instance.enumeratePhysicalDevices();
    check(!devices.empty(), "failed to find GPUs with Vulkan support!");

    auto isDeviceSuitable = [&](vk::PhysicalDevice device)
    {
        if (!QueueFamilyUtils::areAllIndicesSet(QueueFamilyUtils::findQueueFamilies(device, m_surface))
            || !RenderUtils::checkDeviceExtensionSupport(device))
            return false;

        const RenderUtils::SwapChainSupportDetails swapChainSupport = RenderUtils::querySwapChainSupport(
            device, m_surface);
        if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty())
            return false;

        if (const vk::PhysicalDeviceFeatures features = device.getFeatures(); !features.samplerAnisotropy)
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
        fatalError("failed to find a suitable GPU!");
    }
}

bool RenderManager::checkValidationLayerSupport()
{
    const std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

    auto isLayerAvailable = [&](const char* layerName)
    {
        auto matchesName = [&](const vk::LayerProperties& item) { return std::strcmp(layerName, item.layerName) == 0; };
        return std::ranges::any_of(availableLayers, matchesName);
    };

    return std::ranges::all_of(RenderUtils::ValidationLayers, isLayerAvailable);
}

void RenderManager::drawFrame()
{
    const vk::Fence fence = m_inFlightFences[m_currentFrame];
    const vk::Semaphore imageAvailableSemaphore = m_imageAvailableSemaphores[m_currentFrame];
    const vk::Semaphore renderFinishedSemaphore = m_renderFinishedSemaphores[m_currentFrame];
    const vk::CommandBuffer commandBuffer = m_commandBuffers[m_currentFrame];

    if (m_device.waitForFences(1, &fence, vk::False, std::numeric_limits<UInt64>::max()) != vk::Result::eSuccess)
    {
        fatalError("failed to wait for fences!");
    }

    const auto imageResult = m_device.acquireNextImageKHR(m_swapChain, std::numeric_limits<UInt64>::max(), imageAvailableSemaphore, nullptr);

    if (imageResult.result == vk::Result::eErrorOutOfDateKHR)
    {
        recreateSwapchain();
        return;
    }
    else if (imageResult.result != vk::Result::eSuccess && imageResult.result != vk::Result::eSuboptimalKHR)
    {
        fatalError("failed to acquire swap chain image!");
    }

    if (m_device.resetFences(1, &fence) != vk::Result::eSuccess)
    {
        fatalError("failed to reset fences!");
    }

    m_imguiHelper.drawFrame();

    commandBuffer.reset();

    commandBuffer.begin(vk::CommandBufferBeginInfo{});

    const vk::Viewport viewport
    {
        .x = 0,
        .y = 0,
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

    const vk::RenderingAttachmentInfo colorAttachmentInfo
    {
        .imageView = m_swapChainImageViews[m_currentFrame],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = vk::ClearColorValue{0.0f, 0.0f, 0.0f, 1.0f}
    };

    const vk::RenderingAttachmentInfo depthAttachmentInfo
    {
        .imageView = m_depthImageView,
        .imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eDontCare,
        .clearValue = vk::ClearDepthStencilValue{1.0f, 0}
    };

    const vk::RenderingInfo renderingInfo
    {
        .renderArea = vk::Rect2D{{0, 0}, m_swapchainExtent},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentInfo,
        .pDepthAttachment = &depthAttachmentInfo,
    };

    RenderUtils::transitionImageLayout
    (
        commandBuffer,
        m_swapChainImages[imageResult.value],
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        {},
        vk::AccessFlagBits::eColorAttachmentWrite,
        vk::PipelineStageFlagBits::eTopOfPipe,
        vk::PipelineStageFlagBits::eColorAttachmentOutput
    );

    commandBuffer.beginRendering(renderingInfo);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_graphicsPipeline);
    m_renderObjectManager.renderFrame(commandBuffer, m_pipelineLayout, m_currentFrame);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_linePipeline);
    m_renderObjectManager.renderLineFrame(commandBuffer, m_pipelineLayout, m_currentFrame);

    m_imguiHelper.renderFrame(commandBuffer);

    commandBuffer.endRendering();

    RenderUtils::transitionImageLayout
    (
        commandBuffer,
        m_swapChainImages[imageResult.value],
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits::eColorAttachmentWrite,
        {},
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eBottomOfPipe
    );
    
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
        fatalError("failed to submit draw command buffer!");
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
        shouldRecreateSwapchain = result == vk::Result::eSuboptimalKHR || m_framebufferResized.load();
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

std::vector<const char*> RenderManager::getRequiredExtensions()
{
    UInt32 glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if constexpr (vk::EnableValidationLayers)
    {
        extensions.push_back(vk::EXTDebugUtilsExtensionName);
    }

    return extensions;
}

void RenderManager::createDepthResources()
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

    m_depthImageView = RenderUtils::createImageView(m_device, m_depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth);

    RenderUtils::transitionImageLayout(m_device, m_graphicsQueue, m_commandPool, m_depthImage, depthFormat,
                                       vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
}
