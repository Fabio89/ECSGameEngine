module Engine:ImGui;
import :Ecs;
import :ImGui;
import :Render.QueueFamily;
import Wrapper.ImGui;

void ImGuiHelper::init(GLFWwindow* window, const ImGuiInitInfo& initInfo)
{
    if constexpr (!enabled)
        return;

    m_device = initInfo.device;

    // Setup Dear ImGui context
    Wrapper_ImGui::CheckVersion();
    Wrapper_ImGui::CreateContext();
    Wrapper_ImGui::ImGuiIO& io = Wrapper_ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= Wrapper_ImGui::ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= Wrapper_ImGui::ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

    // Setup Dear ImGui style
    Wrapper_ImGui::StyleColorsDark();

    // Init ImGui
    Wrapper_ImGui::ImGui_ImplGlfw_InitForVulkan(window, true);

    static constexpr std::array imguiPoolSizes
    {
        vk::DescriptorPoolSize{vk::DescriptorType::eCombinedImageSampler, 1},
    };

    static constexpr vk::DescriptorPoolCreateInfo imguiPoolInfo
    {
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = static_cast<uint32_t>(MaxFramesInFlight),
        .poolSizeCount = static_cast<uint32_t>(imguiPoolSizes.size()),
        .pPoolSizes = imguiPoolSizes.data(),
    };

    m_descriptorPool = initInfo.device.createDescriptorPool(imguiPoolInfo, nullptr);
    check(m_descriptorPool, "failed to create descriptor pool!");

    Wrapper_ImGui::ImGui_ImplVulkan_InitInfo imguiInitInfo
    {
        .Instance = initInfo.instance,
        .PhysicalDevice = initInfo.physicalDevice,
        .Device = initInfo.device,
        .QueueFamily = *QueueFamilyUtils::findQueueFamilies(initInfo.physicalDevice, initInfo.surface).get(QueueFamilyType::Graphics),
        .Queue = initInfo.queue,
        .DescriptorPool = m_descriptorPool,
        .RenderPass = initInfo.renderPass,
        .MinImageCount = 2,
        .ImageCount = static_cast<uint32_t>(initInfo.imageCount),
        .MSAASamples = Wrapper_ImGui::VK_SAMPLE_COUNT_1_BIT,
        .PipelineCache = initInfo.pipelineCache,
        .Subpass = 0,
        .Allocator = nullptr,
        .CheckVkResultFn = nullptr,
    };
    Wrapper_ImGui::ImGui_ImplVulkan_Init(&imguiInitInfo);
}

void ImGuiHelper::drawFrame()
{
    if constexpr (!enabled)
        return;

    Wrapper_ImGui::ImGui_ImplVulkan_NewFrame();
    Wrapper_ImGui::ImGui_ImplGlfw_NewFrame();
    Wrapper_ImGui::NewFrame();

    for (auto& widget : m_debugWidgets)
    {
        widget->draw();
    }
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void ImGuiHelper::renderFrame(vk::CommandBuffer commandBuffer)
{
    if constexpr (!enabled)
        return;

    Wrapper_ImGui::Render();
    Wrapper_ImGui::ImGui_ImplVulkan_RenderDrawData(Wrapper_ImGui::GetDrawData(), commandBuffer);
}

void ImGuiHelper::shutdown()
{
    if constexpr (!enabled)
        return;

    Wrapper_ImGui::ImGui_ImplVulkan_Shutdown();
    Wrapper_ImGui::ImGui_ImplGlfw_Shutdown();
    Wrapper_ImGui::DestroyContext();

    m_device.destroy(m_descriptorPool);
    m_descriptorPool = nullptr;
}
