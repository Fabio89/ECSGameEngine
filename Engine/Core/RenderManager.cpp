module;

#include <experimental/scope>

module Render.RenderManager;
import Geometry;
import Glfw;
import Guid;
import Render.Pipeline.Line;
import Render.Pipeline.Mesh;
import Render.QueueFamily;
import Render.TextureLoading;
import Render.Utils;
import Render.Vulkan;

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
        .maxSets = 1000, // TODO: figure this out
        .poolSizeCount = static_cast<UInt32>(poolSizes.size()),
        .pPoolSizes = poolSizes.data(),
    };

    return device.createDescriptorPool(poolInfo, nullptr);
}

RenderManager::RenderManager()
    : m_renderWorldManager{m_context, {.descriptorPool = m_descriptorPool, .descriptorSetLayout = m_descriptorSetLayout}},
      m_viewportManager{m_context},
      m_commandProcessor{{.renderWorldManager = m_renderWorldManager}} {}

RenderManager::~RenderManager() noexcept
{
    if (!m_terminated)
    {
        shutdown();
    }
}

void RenderManager::init(WindowHandle window)
{
    // Init window
    check(!m_initialised, "[RenderManager] Tried to initialise more than once!");
    check(!m_window, "[RenderManager] Tried to create a new window without having deleted the current one!");
    check(window.isValid(), "[RenderManager] Can't initialise without a window!");

    m_window = window;

    // Init Vulkan
    {
        vk::detail::defaultDispatchLoaderDynamic.init();

        // Determine what API version is available
        const UInt32 apiVersion = vk::enumerateInstanceVersion();
        std::cout << "Loader/Runtime support detected for Vulkan " << vk::apiVersionMajor(apiVersion) << "." <<
                vk::apiVersionMinor(apiVersion) << "\n";

        createInstance();

        RenderUtils::createDebugUtilsMessenger(m_context.instance, &m_debugMessenger, nullptr);
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapchain();
        m_descriptorPool = createDescriptorPool(m_context.device);
        m_descriptorSetLayout = createDescriptorSetLayout(m_context.device);
        m_pipelineLayout = createPipelineLayout(m_context.device, m_descriptorSetLayout);

        constexpr GraphicsPipelineConfig mainPipelineConfig{};
        m_graphicsPipeline = createGraphicsPipeline(m_context.device, m_pipelineCache, m_pipelineLayout, mainPipelineConfig);

        constexpr GraphicsPipelineConfig gizmoPipelineConfig
        {
            .cullMode = vk::CullModeFlagBits::eNone,
            .depthTest = false,
            .depthWrite = false,
            .blending = true
        };
        m_gizmoPipeline = createGraphicsPipeline(m_context.device, m_pipelineCache, m_pipelineLayout, gizmoPipelineConfig);

        m_linePipeline = createLinePipeline(m_context.device, m_pipelineCache, m_pipelineLayout);

        createCommandPool();
        createCommandBuffers();
        createSyncObjects();
    }

    const ImGuiInitInfo imguiInfo
    {
        .instance = m_context.instance,
        .physicalDevice = m_context.physicalDevice,
        .device = m_context.device,
        .surface = m_context.surface,
        .queue = m_context.graphicsQueue,
        .imageCount = m_swapchain.imageViews.size(),
        .pipelineCache = m_pipelineCache,
        .colorFormat = {vk::Format::eB8G8R8A8Srgb},
        .depthFormat = RenderUtils::findDepthFormat(m_context.physicalDevice),
    };
    m_imguiHelper.init(window, imguiInfo);

    if (m_editorCallbacks.imguiInit)
        m_editorCallbacks.imguiInit();

    GLFWwindow* glfwWindow = Platform::Window::getGlfwWindow(window);
    glfwSetWindowUserPointer(glfwWindow, this);
    glfwSetFramebufferSizeCallback(glfwWindow, [](GLFWwindow* w, int, int)
    {
        auto* self = static_cast<RenderManager*>(glfwGetWindowUserPointer(w));
        self->m_framebufferResized = true;
    });

    m_initialised = true;
}

void RenderManager::update()
{
    {
        std::lock_guard lock{m_updateLockMutex};

        m_context.graphicsQueue.waitIdle(); // TODO(perf): Explore VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT.

        m_viewportManager.update();

        m_commandProcessor.processAll();
    }

    drawFrame();
    m_frameTimer.tick();
}

void RenderManager::shutdown()
{
    std::lock_guard lock{m_updateLockMutex};

    m_terminated = true;
    m_context.device.waitIdle();

    m_renderWorldManager.clear();

    m_imguiHelper.shutdown();

    cleanupSwapchain();
    m_viewportManager.shutdown();

    m_context.device.destroyDescriptorPool(m_descriptorPool);
    m_context.device.destroyDescriptorSetLayout(m_descriptorSetLayout);
    m_context.device.destroyCommandPool(m_context.commandPool);
    m_context.device.destroyCommandPool(m_transferCommandPool);
    m_context.device.destroyPipeline(m_graphicsPipeline);
    m_context.device.destroyPipeline(m_gizmoPipeline);
    m_context.device.destroyPipeline(m_linePipeline);
    m_context.device.destroyPipelineLayout(m_pipelineLayout);

    for (std::size_t i = 0; i < MaxFramesInFlight; ++i)
    {
        m_context.device.destroySemaphore(m_imageAvailableSemaphores[i]);
        m_context.device.destroySemaphore(m_renderFinishedSemaphores[i]);
        m_context.device.destroyFence(m_inFlightFences[i]);
    }

    m_context.device.destroy();
    m_context.instance.destroySurfaceKHR(m_context.surface);

    if constexpr (vk::EnableValidationLayers)
    {
        RenderUtils::destroyDebugUtilsMessenger(m_context.instance, m_debugMessenger, nullptr);
    }

    m_context.instance.destroy();
}

void RenderManager::clear()
{
    if (!m_initialised)
        return;

    m_context.device.waitIdle();
    m_renderWorldManager.clear();
}

void RenderManager::updateFramebufferSize()
{
    m_framebufferResized = true;
}

void RenderManager::setEditorCallbacks(EditorCallbacks callbacks)
{
    m_editorCallbacks = std::move(callbacks);
}

ViewportId RenderManager::createViewport(std::span<WorldHandle> worlds, Rect area)
{
    std::vector<std::reference_wrapper<RenderWorld>> renderWorlds;
    renderWorlds.reserve(worlds.size());
    for (const WorldHandle handle : worlds)
        renderWorlds.push_back(m_renderWorldManager.get(handle));

    return m_viewportManager.createViewport({
        .requestedArea = area,
        .colorFormat = m_swapchain.imageFormat,
        .renderWorlds = std::move(renderWorlds)
    });
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

    m_context.instance = vk::createInstance(createInfo);

    vk::detail::defaultDispatchLoaderDynamic.init(m_context.instance);
}

void RenderManager::createSurface()
{
    check(glfw::createWindowSurface(m_context.instance, Platform::Window::getGlfwWindow(m_window), nullptr, &m_context.surface) == vk::Result::eSuccess, "Failed to create window surface!");
}

void RenderManager::createLogicalDevice()
{
    const QueueFamilyIndices indices = QueueFamilyUtils::findQueueFamilies(m_context.physicalDevice, m_context.surface);

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;

    queueCreateInfos.reserve(indices.size());

    auto indexRange = indices | std::views::transform([&](auto&& index) { return *index; });
    std::set<std::optional<UInt32> > uniqueQueueFamilies(indexRange.begin(), indexRange.end());

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

    m_context.device = m_context.physicalDevice.createDevice(createInfo);
    check(m_context.device, "failed to create logical device!");

    m_context.graphicsQueue = m_context.device.getQueue(*indices.get(QueueFamilyType::Graphics), 0);
    m_presentQueue = m_context.device.getQueue(*indices.get(QueueFamilyType::Present), 0);
    m_transferQueue = m_context.device.getQueue(*indices.get(QueueFamilyType::Transfer), 0);
}

void RenderManager::recreateSwapchain()
{
    int width = 0, height = 0;
    GLFWwindow* glfwWindow = Platform::Window::getGlfwWindow(m_window);
    glfwGetFramebufferSize(glfwWindow, &width, &height);
    while (width == 0 || height == 0) // NOLINT(bugprone-infinite-loop)
    {
        glfwGetFramebufferSize(glfwWindow, &width, &height);
        glfwWaitEvents();
    }

    m_context.device.waitIdle();

    cleanupSwapchain();
    createSwapchain();

    m_viewportManager.recreateImages();

    m_imguiHelper.recreateSwapchain(m_swapchain.images.size());
}

void RenderManager::createSwapchain()
{
    const RenderUtils::SwapChainSupportDetails swapChainSupport = RenderUtils::querySwapChainSupport(m_context.physicalDevice, m_context.surface);

    const vk::SurfaceFormatKHR surfaceFormat = RenderUtils::chooseSwapSurfaceFormat(swapChainSupport.formats);
    const vk::PresentModeKHR presentMode = RenderUtils::chooseSwapPresentMode(swapChainSupport.presentModes);
    const vk::Extent2D extent = RenderUtils::chooseSwapExtent(swapChainSupport.capabilities, Platform::Window::getGlfwWindow(m_window));

    UInt32 minImageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && minImageCount > swapChainSupport.capabilities.maxImageCount)
    {
        minImageCount = swapChainSupport.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo
    {
        .surface = m_context.surface,
        .minImageCount = minImageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst,
        .preTransform = swapChainSupport.capabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = presentMode,
        .clipped = vk::False,
        .oldSwapchain = nullptr,
    };

    const QueueFamilyIndices indices = QueueFamilyUtils::findQueueFamilies(m_context.physicalDevice, m_context.surface);
    auto indicesRange = indices | std::views::transform([](std::optional<UInt32> value) { return *value; });
    const std::vector<UInt32> queueFamilyIndices(indicesRange.begin(), indicesRange.end());

    if (indices.get(QueueFamilyType::Graphics) != indices.get(QueueFamilyType::Present))
    {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    } else
    {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    m_swapchain.handle = m_context.device.createSwapchainKHR(createInfo, nullptr);
    if (!m_swapchain.handle)
    {
        fatalError("failed to create swap chain!");
    }

    m_swapchain.images = m_context.device.getSwapchainImagesKHR(m_swapchain.handle);
    m_swapchain.layouts.assign( m_swapchain.images.size(), vk::ImageLayout::eUndefined);
    m_swapchain.imageFormat = surfaceFormat.format;
    m_swapchain.extent = extent;

    m_swapchain.imageViews.resize(m_swapchain.images.size());

    for (std::size_t i = 0; i < m_swapchain.images.size(); ++i)
    {
        m_swapchain.imageViews[i] = RenderUtils::createImageView(m_context.device, m_swapchain.images[i], m_swapchain.imageFormat);
    }
}

void RenderManager::cleanupSwapchain()
{
    for (vk::ImageView imageView : m_swapchain.imageViews)
        m_context.device.destroyImageView(imageView);
    m_swapchain.imageViews.clear();

    m_context.device.destroySwapchainKHR(m_swapchain.handle);
    m_swapchain.handle = nullptr;

    m_swapchain.images.clear();
    m_swapchain.layouts.clear();
}

void RenderManager::createCommandPool()
{
    const QueueFamilyIndices queueFamilyIndices = QueueFamilyUtils::findQueueFamilies(m_context.physicalDevice, m_context.surface);

    const vk::CommandPoolCreateInfo poolInfo
    {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = *queueFamilyIndices.get(QueueFamilyType::Graphics),
    };

    m_context.commandPool = m_context.device.createCommandPool(poolInfo, nullptr);
    if (!m_context.commandPool)
    {
        fatalError("failed to create command pool!");
    }

    const vk::CommandPoolCreateInfo transferPoolInfo
    {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = *queueFamilyIndices.get(QueueFamilyType::Transfer),
    };

    m_transferCommandPool = m_context.device.createCommandPool(transferPoolInfo, nullptr);

    if (!m_transferCommandPool)
    {
        fatalError("failed to create transfer command pool!");
    }
}

void RenderManager::createCommandBuffers()
{
    const vk::CommandBufferAllocateInfo allocInfo
    {
        .commandPool = m_context.commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = static_cast<UInt32>(MaxFramesInFlight),
    };

    m_commandBuffers = m_context.device.allocateCommandBuffers(allocInfo);
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
        imageAvailable = m_context.device.createSemaphore(semaphoreInfo, nullptr);
        renderFinished = m_context.device.createSemaphore(semaphoreInfo, nullptr);
        fence = m_context.device.createFence(fenceInfo, nullptr);

        if (!imageAvailable || !renderFinished || !fence)
        {
            fatalError("failed to create semaphores!");
        }
    }
}

void RenderManager::pickPhysicalDevice()
{
    const std::vector<vk::PhysicalDevice> devices = m_context.instance.enumeratePhysicalDevices();
    check(!devices.empty(), "failed to find GPUs with Vulkan support!");

    auto isDeviceSuitable = [&](vk::PhysicalDevice device)
    {
        if (!QueueFamilyUtils::areAllIndicesSet(QueueFamilyUtils::findQueueFamilies(device, m_context.surface))
            || !RenderUtils::checkDeviceExtensionSupport(device))
            return false;

        const RenderUtils::SwapChainSupportDetails swapChainSupport = RenderUtils::querySwapChainSupport(
            device, m_context.surface);
        if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty())
            return false;

        if (const vk::PhysicalDeviceFeatures features = device.getFeatures(); !features.samplerAnisotropy)
            return false;
        return true;
    };

    auto found = std::ranges::find_if(devices, isDeviceSuitable);

    if (found != devices.end())
    {
        m_context.physicalDevice = *found;
        vk::PhysicalDeviceProperties properties = m_context.physicalDevice.getProperties();
        std::cout << "Using device: " << properties.deviceName << "\n";
    } else
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
    //--------------------------------------------------------------------------
    // Build ImGui draw data
    //--------------------------------------------------------------------------
    if (m_editorCallbacks.draw)
    {
        m_imguiHelper.beginFrame();
        m_editorCallbacks.draw();
        m_imguiHelper.preRenderFrame();
    }

    //--------------------------------------------------------------------------
    // Acquire next swapchain image and begin command buffer
    //--------------------------------------------------------------------------
    const vk::Fence fence = m_inFlightFences[m_currentFrame];
    const vk::Semaphore imageAvailableSemaphore = m_imageAvailableSemaphores[m_currentFrame];
    const vk::Semaphore renderFinishedSemaphore = m_renderFinishedSemaphores[m_currentFrame];
    const vk::CommandBuffer commandBuffer = m_commandBuffers[m_currentFrame];

    if (m_context.device.waitForFences(1, &fence, vk::False, std::numeric_limits<UInt64>::max()) != vk::Result::eSuccess)
    {
        fatalError("failed to wait for fences!");
    }

    const auto imageResult = m_context.device.acquireNextImageKHR(m_swapchain.handle, std::numeric_limits<UInt64>::max(), imageAvailableSemaphore, nullptr);

    if (imageResult.result == vk::Result::eErrorOutOfDateKHR)
    {
        recreateSwapchain();
        return;
    }

    if (imageResult.result != vk::Result::eSuccess && imageResult.result != vk::Result::eSuboptimalKHR)
        fatalError("failed to acquire swap chain image!");

    if (m_context.device.resetFences(1, &fence) != vk::Result::eSuccess)
        fatalError("failed to reset fences!");

    commandBuffer.reset();
    commandBuffer.begin(vk::CommandBufferBeginInfo{});

    const UInt32 imageIndex = imageResult.value;

    //--------------------------------------------------------------------------
    // Prepare swapchain image
    //--------------------------------------------------------------------------
    RenderUtils::transitionImageLayout
    (
        commandBuffer,
        m_swapchain.images[imageIndex],
        m_swapchain.layouts[imageIndex],
        vk::ImageLayout::eTransferDstOptimal,
        {},
        vk::AccessFlagBits::eTransferWrite,
        vk::PipelineStageFlagBits::eTopOfPipe,
        vk::PipelineStageFlagBits::eTransfer
    );
    m_swapchain.layouts[imageIndex] = vk::ImageLayout::eTransferDstOptimal;

    {
        static constexpr std::array ranges
        {
            vk::ImageSubresourceRange
            {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            }
        };

        static constexpr vk::ClearColorValue clearColor{0.f, 0.f, 0.f, 1.f};

        commandBuffer.clearColorImage(m_swapchain.images[imageIndex], vk::ImageLayout::eTransferDstOptimal, clearColor, ranges);
    }

    const RenderPassContext renderContext
    {
        .commandBuffer = commandBuffer,
        .pipelines = {.mesh = m_graphicsPipeline, .gizmo = m_gizmoPipeline, .line = m_linePipeline, .layout = m_pipelineLayout},
        .frameIndex = m_currentFrame,
        .imageIndex = static_cast<Int32>(imageIndex),
        .destination = {
            .image = m_swapchain.images[imageIndex],
            .view = m_swapchain.imageViews[imageIndex],
            .layout = &m_swapchain.layouts[imageIndex],
            .extent = m_swapchain.extent,
            .format = m_swapchain.imageFormat
        }
    };

    m_viewportManager.drawViewports(renderContext);

    //--------------------------------------------------------------------------
    // Render ImGui over the swapchain
    //--------------------------------------------------------------------------
    RenderUtils::transitionImageLayout
    (
        commandBuffer,
        m_swapchain.images[imageIndex],
        m_swapchain.layouts[imageIndex],
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::AccessFlagBits::eTransferWrite,
        vk::AccessFlagBits::eColorAttachmentWrite,
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eColorAttachmentOutput
    );
    m_swapchain.layouts[imageIndex] = vk::ImageLayout::eColorAttachmentOptimal;

    const vk::RenderingAttachmentInfo imGuiColorAttachmentInfo
    {
        .imageView = m_swapchain.imageViews[imageIndex],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eLoad,
        .storeOp = vk::AttachmentStoreOp::eStore,
    };

    const vk::RenderingInfo imGuiRenderingInfo
    {
        .renderArea = {{0, 0}, m_swapchain.extent},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &imGuiColorAttachmentInfo,
    };

    commandBuffer.beginRendering(imGuiRenderingInfo);
    m_imguiHelper.renderFrame(commandBuffer);
    commandBuffer.endRendering();

    //--------------------------------------------------------------------------
    // Transition to present
    //--------------------------------------------------------------------------
    RenderUtils::transitionImageLayout
    (
        commandBuffer,
        m_swapchain.images[imageIndex],
        m_swapchain.layouts[imageIndex],
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits::eColorAttachmentWrite,
        {},
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eBottomOfPipe
    );
    m_swapchain.layouts[imageIndex] = vk::ImageLayout::ePresentSrcKHR;

    commandBuffer.end();

    //--------------------------------------------------------------------------
    // Submit
    //--------------------------------------------------------------------------
    const vk::Semaphore waitSemaphores[] = { imageAvailableSemaphore };
    const vk::Semaphore signalSemaphores[] = { renderFinishedSemaphore };
    static constexpr vk::PipelineStageFlags waitStages[]
    {
        vk::PipelineStageFlagBits::eColorAttachmentOutput
    };

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

    if (m_context.graphicsQueue.submit(1, &submitInfo, fence) != vk::Result::eSuccess)
    {
        fatalError("failed to submit draw command buffer!");
    }

    //--------------------------------------------------------------------------
    // Present
    //--------------------------------------------------------------------------
    vk::SwapchainKHR swapChains[] = { m_swapchain.handle };

    const vk::PresentInfoKHR presentInfo
    {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
        .swapchainCount = 1,
        .pSwapchains = swapChains,
        .pImageIndices = &imageIndex,
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
