module;
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
module Engine.ImGui;

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
        .QueueFamily = *QueueFamilyUtils::findQueueFamilies(initInfo.physicalDevice, initInfo.surface).get(QueueFamilyType::Graphics),
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
