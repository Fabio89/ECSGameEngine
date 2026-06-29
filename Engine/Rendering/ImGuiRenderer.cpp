module Render.ImGui;
import Render.QueueFamily;
import Editor.ImGui;
import Editor.PropertyDrawers;

static void check_vk_result(ImGui::VkResult result)
{
    if (result != ImGui::VkResult::VK_SUCCESS)
        log(std::format("ImGui Vulkan error: {}\n", static_cast<int>(result)));
}

void ImGuiRenderer::init(WindowHandle window, const ImGuiInitInfo& initInfo)
{
    if constexpr (!enabled)
        return;

    UI::initPropertyDrawers();

    m_device = initInfo.device;

    // Setup Dear ImGui context
    ImGui::CheckVersion();
    ImGui::CreateContext();
    ImGui::ImGuiIO& io = ImGui::GetIO();

    // Set fonts
    {
        ImGui::ImFontConfig config;
        config.OversampleH = 2;
        config.OversampleV = 2;

        io.Fonts->AddFontFromFileTTF(
            "Editor/Assets/Fonts/NotoSans-Variable.ttf",
            18.0f,
            &config
        );
        io.FontDefault = io.Fonts->Fonts.back();
    }

    std::cout << "BackendRendererName="
          << (io.BackendRendererName ? io.BackendRendererName : "null")
          << "\n";

    std::cout << "BackendFlags="
              << io.BackendFlags
              << "\n";

    (void)io;
    io.ConfigFlags |= ImGui::ImGuiConfigFlags_::ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGui::ImGuiConfigFlags_::ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGui::ImGuiConfigFlags_::ImGuiConfigFlags_DockingEnable;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Init ImGui
    ImGui::ImGui_ImplGlfw_InitForVulkan(Platform::Window::getGlfwWindow(window), true);

    static constexpr std::array imguiPoolSizes
    {
        vk::DescriptorPoolSize{vk::DescriptorType::eSampler, 100},
        vk::DescriptorPoolSize{vk::DescriptorType::eCombinedImageSampler, 100},
        vk::DescriptorPoolSize{vk::DescriptorType::eSampledImage, 100},
        vk::DescriptorPoolSize{vk::DescriptorType::eStorageImage, 100},
        vk::DescriptorPoolSize{vk::DescriptorType::eUniformTexelBuffer, 100},
        vk::DescriptorPoolSize{vk::DescriptorType::eStorageTexelBuffer, 100},
        vk::DescriptorPoolSize{vk::DescriptorType::eUniformBuffer, 100},
        vk::DescriptorPoolSize{vk::DescriptorType::eStorageBuffer, 100},
        vk::DescriptorPoolSize{vk::DescriptorType::eUniformBufferDynamic, 100},
        vk::DescriptorPoolSize{vk::DescriptorType::eStorageBufferDynamic, 100},
        vk::DescriptorPoolSize{vk::DescriptorType::eInputAttachment, 100},
    };

    static constexpr vk::DescriptorPoolCreateInfo imguiPoolInfo
    {
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = 1000,
        .poolSizeCount = static_cast<UInt32>(imguiPoolSizes.size()),
        .pPoolSizes = imguiPoolSizes.data(),
    };

    m_descriptorPool = initInfo.device.createDescriptorPool(imguiPoolInfo, nullptr);
    check(m_descriptorPool, "failed to create descriptor pool!");

    const vk::PipelineRenderingCreateInfo renderingCreateInfo
    {
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &initInfo.colorFormat,
        .depthAttachmentFormat = initInfo.depthFormat,
        .stencilAttachmentFormat = vk::Format::eUndefined
    };

    ImGui::ImGui_ImplVulkan_PipelineInfo pipelineInfo
    {
        .MSAASamples = ImGui::VK_SAMPLE_COUNT_1_BIT,
        .PipelineRenderingCreateInfo = renderingCreateInfo
    };

    ImGui::ImGui_ImplVulkan_InitInfo imguiInitInfo
    {
        .Instance = initInfo.instance,
        .PhysicalDevice = initInfo.physicalDevice,
        .Device = initInfo.device,
        .QueueFamily = *QueueFamilyUtils::findQueueFamilies(initInfo.physicalDevice, initInfo.surface).get(QueueFamilyType::Graphics),
        .Queue = initInfo.queue,
        .DescriptorPool = m_descriptorPool,
        .MinImageCount = static_cast<UInt32>(initInfo.imageCount),
        .ImageCount = static_cast<UInt32>(initInfo.imageCount),
        .PipelineCache = initInfo.pipelineCache,
        .PipelineInfoMain = pipelineInfo,
        .UseDynamicRendering = true,
        .Allocator = nullptr,
        .CheckVkResultFn = check_vk_result,
    };
    ImGui::ImGui_ImplVulkan_Init(&imguiInitInfo);
}

void ImGuiRenderer::beginFrame()
{
    if constexpr (!enabled)
        return;

    ImGui::ImGui_ImplVulkan_NewFrame();
    ImGui::ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiRenderer::preRenderFrame()
{
    if constexpr (!enabled)
        return;
    ImGui::Render();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void ImGuiRenderer::renderFrame(vk::CommandBuffer commandBuffer)
{
    if constexpr (!enabled)
        return;

    ImGui::ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

void ImGuiRenderer::recreateSwapchain(std::size_t newSize)
{
    ImGui::ImGui_ImplVulkan_SetMinImageCount(static_cast<UInt32>(newSize));
}

void ImGuiRenderer::shutdown()
{
    if constexpr (!enabled)
        return;

    ImGui::ImGui_ImplVulkan_Shutdown();
    ImGui::ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    m_device.destroy(m_descriptorPool);
    m_descriptorPool = nullptr;
}
