module;
#include <cstddef>

export module Render.Pipeline.MeshWithTexture;
import Core;
import Render.Model;
import Render.Utils;
import Project;
import Wrapper.Vulkan;

export vk::Pipeline createGraphicsPipeline(vk::Device device, vk::PipelineCache pipelineCache, vk::PipelineLayout pipelineLayout, vk::Extent2D swapchainExtent)
{
    static constexpr std::array dynamicStates
    {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    static constexpr vk::PipelineDynamicStateCreateInfo dynamicStateInfo
    {
        .dynamicStateCount = static_cast<UInt32>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data(),
    };

    static constexpr vk::VertexInputBindingDescription bindingDescription
    {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = vk::VertexInputRate::eVertex,
    };

    static constexpr std::array attributeDescriptions
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
            .offset = offsetof(Vertex, uv),
        },
    };

    static constexpr vk::PipelineVertexInputStateCreateInfo vertexInputInfo
    {
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<UInt32>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions = attributeDescriptions.data(),
    };

    static constexpr vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo
    {
        .topology = vk::PrimitiveTopology::eTriangleList,
        .primitiveRestartEnable = vk::False,
    };

    static constexpr vk::PipelineRasterizationStateCreateInfo rasterizerInfo
    {
        .depthClampEnable = vk::False,
        .rasterizerDiscardEnable = vk::False,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eClockwise,
        .depthBiasEnable = vk::False,
        .lineWidth = 1.0f,
    };

    static constexpr vk::PipelineMultisampleStateCreateInfo multisamplingInfo
    {
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = vk::False,
    };

    static constexpr vk::PipelineColorBlendAttachmentState colorBlendAttachment
    {
        .blendEnable = vk::False,
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
    };

    static constexpr vk::PipelineColorBlendStateCreateInfo colorBlendingInfo
    {
        .logicOpEnable = vk::False,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants = std::array{0.f, 0.f, 0.f, 0.f}
    };

    static constexpr vk::PipelineDepthStencilStateCreateInfo depthStencil
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

    auto vertShaderCode = RenderUtils::readFile(RenderUtils::getExecutableRoot() + "Shaders/vert.spv");
    auto fragShaderCode = RenderUtils::readFile(RenderUtils::getExecutableRoot() + "Shaders/frag.spv");

    vk::ShaderModule vertShaderModule = RenderUtils::createShaderModule(vertShaderCode, device);
    vk::ShaderModule fragShaderModule = RenderUtils::createShaderModule(fragShaderCode, device);

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

    const vk::Viewport viewport
    {
        .x = 0,
        .y = 0,
        .width = static_cast<float>(swapchainExtent.width),
        .height = static_cast<float>(swapchainExtent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    const vk::Rect2D scissor
    {
        .offset = {0, 0},
        .extent = swapchainExtent,
    };

    const vk::PipelineViewportStateCreateInfo viewportStateInfo
    {
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor
    };

    static constexpr vk::Format colorFormat{vk::Format::eB8G8R8A8Srgb};
    static constexpr vk::Format depthFormat{vk::Format::eD32Sfloat};
    
    static constexpr vk::PipelineRenderingCreateInfo renderingCreateInfo
    {
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &colorFormat,
        .depthAttachmentFormat = depthFormat,
        .stencilAttachmentFormat = vk::Format::eUndefined
    };
    
    const vk::GraphicsPipelineCreateInfo pipelineInfo
    {
        .pNext = &renderingCreateInfo,
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
        .layout = pipelineLayout,
        .renderPass = nullptr,
        .subpass = 0,
        .basePipelineHandle = nullptr,
        .basePipelineIndex = -1,
    };

    auto&& [result, pipeline] = device.createGraphicsPipeline(pipelineCache, pipelineInfo);

    if (result != vk::Result::eSuccess)
    {
        fatalError("failed to create graphics pipeline!");
    }

    device.destroyShaderModule(fragShaderModule, nullptr);
    device.destroyShaderModule(vertShaderModule, nullptr);

    return pipeline;
}
