module;
#include <cstddef>

export module Render.Pipeline.Mesh;
import Assets.Mesh;
import Core;
import Render.Utils;
import Render.Vulkan;

export struct GraphicsPipelineConfig
{
    vk::CullModeFlags cullMode{vk::CullModeFlagBits::eBack};
    vk::FrontFace frontFace{vk::FrontFace::eClockwise};
    bool depthTest{true};
    bool depthWrite{true};
    vk::CompareOp depthCompareOp{vk::CompareOp::eLess};
    bool blending{false};
    vk::PrimitiveTopology topology{vk::PrimitiveTopology::eTriangleList};
};

export vk::Pipeline createGraphicsPipeline(vk::Device device, vk::PipelineCache pipelineCache, vk::PipelineLayout pipelineLayout, const GraphicsPipelineConfig& config)
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
        }
    };

    static constexpr vk::PipelineVertexInputStateCreateInfo vertexInputInfo
    {
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<UInt32>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions = attributeDescriptions.data(),
    };

    const vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo
    {
        .topology = config.topology,
        .primitiveRestartEnable = vk::False,
    };

    const vk::PipelineRasterizationStateCreateInfo rasterizerInfo
    {
        .depthClampEnable = vk::False,
        .rasterizerDiscardEnable = vk::False,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = config.cullMode,
        .frontFace = config.frontFace,
        .depthBiasEnable = vk::False,
        .lineWidth = 1.0f,
    };

    static constexpr vk::PipelineMultisampleStateCreateInfo multisamplingInfo
    {
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = vk::False,
    };

    const vk::PipelineColorBlendAttachmentState colorBlendAttachment
    {
        .blendEnable = config.blending,

        .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
        .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
        .colorBlendOp = vk::BlendOp::eAdd,

        .srcAlphaBlendFactor = vk::BlendFactor::eOne,
        .dstAlphaBlendFactor = vk::BlendFactor::eZero,
        .alphaBlendOp = vk::BlendOp::eAdd,

        .colorWriteMask = vk::ColorComponentFlagBits::eR |
                          vk::ColorComponentFlagBits::eG |
                          vk::ColorComponentFlagBits::eB |
                          vk::ColorComponentFlagBits::eA,
    };

    const vk::PipelineColorBlendStateCreateInfo colorBlendingInfo
    {
        .logicOpEnable = vk::False,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants = std::array{0.f, 0.f, 0.f, 0.f}
    };

    const vk::PipelineDepthStencilStateCreateInfo depthStencil
    {
        .depthTestEnable = config.depthTest,
        .depthWriteEnable = config.depthWrite,
        .depthCompareOp = config.depthCompareOp,
        .depthBoundsTestEnable = vk::False,
        .stencilTestEnable = vk::False,
        .front = {}, // Optional
        .back = {}, // Optional
        .minDepthBounds = 0.0f, // Optional
        .maxDepthBounds = 1.0f, // Optional
    };

    auto vertShaderCode = RenderUtils::readFile(RenderUtils::getExecutableRoot() / "Shaders/Shader.vert.spv");
    auto fragShaderCode = RenderUtils::readFile(RenderUtils::getExecutableRoot() / "Shaders/Shader.frag.spv");

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

    static constexpr vk::PipelineViewportStateCreateInfo viewportStateInfo
    {
        .viewportCount = 1,
        .scissorCount = 1
    };

    static constexpr auto colorFormat{vk::Format::eB8G8R8A8Srgb};
    static constexpr auto depthFormat{vk::Format::eD32Sfloat};

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
