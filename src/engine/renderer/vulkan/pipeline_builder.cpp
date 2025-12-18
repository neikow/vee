#include "pipeline_builder.h"

#include "vertex_utils.h"
#include "vulkan_device.h"

namespace Vulkan {
    VkPipeline PipelineCache::CreateNewVulkanPipeline(
        const PipelineStateKey &key,
        const std::shared_ptr<VulkanDevice> &device,
        const VkPipelineLayout &layout
    ) {
        VkPipelineShaderStageCreateInfo vertShaderStage{};
        vertShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStage.module = key.vertexShader;
        vertShaderStage.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStage{};
        fragShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStage.module = key.fragmentShader;
        fragShaderStage.pName = "main";

        VkPipelineDepthStencilStateCreateInfo depthStencilState{};
        depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilState.depthTestEnable = key.depthTest;
        depthStencilState.depthWriteEnable = key.depthWrite;
        depthStencilState.depthCompareOp = key.depthCompareOp;
        depthStencilState.depthBoundsTestEnable = VK_FALSE;
        depthStencilState.stencilTestEnable = VK_FALSE;
        depthStencilState.front = {};
        depthStencilState.back = {};
        depthStencilState.minDepthBounds = 0.0f;
        depthStencilState.maxDepthBounds = 1.0f;

        std::array shaderStages = {vertShaderStage, fragShaderStage};

        const std::vector dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        auto bindingDescription = VertexUtils::GetBindingDescription();
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        if (key.vertexLayout == DEFAULT) {
            attributeDescriptions = VertexUtils::GetAttributeDescriptions();
        } else if (key.vertexLayout == POSITION_ONLY) {
            attributeDescriptions = VertexUtils::GetPositionAttributeDescriptions();
        } else {
            throw std::runtime_error("Unsupported vertex layout in pipeline builder");
        }

        VkPipelineVertexInputStateCreateInfo vertexInput{};
        vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInput.vertexBindingDescriptionCount = 1;
        vertexInput.pVertexBindingDescriptions = &bindingDescription;
        vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInput.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = key.topology;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(key.viewportExtent.width);
        viewport.height = static_cast<float>(key.viewportExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = key.viewportExtent;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.polygonMode = key.polygonMode;
        rasterizer.cullMode = key.cullMode;
        rasterizer.frontFace = key.frontFace;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f;
        rasterizer.depthBiasClamp = 0.0f;
        rasterizer.depthBiasSlopeFactor = 0.0f;
        rasterizer.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo multisampleState{};
        multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampleState.sampleShadingEnable = VK_FALSE;
        multisampleState.minSampleShading = 1.0f;
        multisampleState.pSampleMask = nullptr;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.blendEnable = key.blendEnable;
        colorBlendAttachment.srcColorBlendFactor = key.srcColorBlendFactor;
        colorBlendAttachment.dstColorBlendFactor = key.dstColorBlendFactor;
        colorBlendAttachment.colorBlendOp = key.colorBlendOp;
        colorBlendAttachment.srcAlphaBlendFactor = key.srcAlphaBlendFactor;
        colorBlendAttachment.dstAlphaBlendFactor = key.dstAlphaBlendFactor;
        colorBlendAttachment.alphaBlendOp = key.alphaBlendOp;
        colorBlendAttachment.colorWriteMask = key.colorWriteMask;

        VkPipelineColorBlendStateCreateInfo colorBlend{};
        colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlend.logicOpEnable = VK_FALSE;
        colorBlend.logicOp = VK_LOGIC_OP_COPY;
        colorBlend.attachmentCount = 1;
        colorBlend.pAttachments = &colorBlendAttachment;
        colorBlend.blendConstants[0] = 0.0f;
        colorBlend.blendConstants[1] = 0.0f;
        colorBlend.blendConstants[2] = 0.0f;
        colorBlend.blendConstants[3] = 0.0f;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInput;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampleState;
        pipelineInfo.pDepthStencilState = &depthStencilState;
        pipelineInfo.pColorBlendState = &colorBlend;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = layout;
        pipelineInfo.renderPass = key.renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        return device->CreatePipeline(pipelineInfo);
    }

    VkPipeline PipelineCache::GetOrCreatePipeline(
        const PipelineStateKey &key,
        const std::shared_ptr<VulkanDevice> &device,
        const VkPipelineLayout &layout
    ) {
        const auto it = m_Cache.find(key);
        if (it != m_Cache.end()) {
            return it->second;
        }

        const auto newPipeline = CreateNewVulkanPipeline(key, device, layout);
        m_Cache[key] = newPipeline;
        return newPipeline;
    }

    PipelineBuilder &PipelineBuilder::SetShaders(
        const VkShaderModule &vertexShader,
        const VkShaderModule &fragmentShader
    ) {
        m_Key.vertexShader = vertexShader;
        m_Key.fragmentShader = fragmentShader;
        return *this;
    }

    PipelineBuilder &PipelineBuilder::SetTopology(
        const VkPrimitiveTopology topology
    ) {
        m_Key.topology = topology;
        return *this;
    }

    PipelineBuilder &PipelineBuilder::SetDepthState(
        const VkBool32 test,
        const VkBool32 write,
        const VkCompareOp op
    ) {
        m_Key.depthTest = test;
        m_Key.depthWrite = write;
        m_Key.depthCompareOp = op;
        return *this;
    }

    PipelineBuilder &PipelineBuilder::SetRenderPass(
        const VkRenderPass &rp
    ) {
        m_Key.renderPass = rp;
        return *this;
    }

    PipelineBuilder &PipelineBuilder::SetExtent(const VkExtent2D &extent) {
        m_Key.viewportExtent = extent;
        return *this;
    }

    PipelineBuilder &PipelineBuilder::SetVertexLayout(const VertexLayout layout) {
        m_Key.vertexLayout = layout;
        return *this;
    }

    PipelineBuilder &PipelineBuilder::SetRasterizer(
        const VkPolygonMode &polygonMode,
        const VkCullModeFlags &cullMode,
        const VkFrontFace &frontFace
    ) {
        m_Key.polygonMode = polygonMode;
        m_Key.cullMode = cullMode;
        m_Key.frontFace = frontFace;
        return *this;
    }

    PipelineBuilder &PipelineBuilder::SetColorBlending(
        const VkBool32 blendEnable,
        const VkBlendFactor &srcColorBlendFactor,
        const VkBlendFactor &dstColorBlendFactor,
        const VkBlendOp &colorBlendOp,
        const VkBlendFactor &srcAlphaBlendFactor,
        const VkBlendFactor &dstAlphaBlendFactor,
        const VkBlendOp &alphaBlendOp,
        const VkColorComponentFlags &colorWriteMask
    ) {
        m_Key.blendEnable = blendEnable;
        m_Key.srcColorBlendFactor = srcColorBlendFactor;
        m_Key.dstColorBlendFactor = dstColorBlendFactor;
        m_Key.colorBlendOp = colorBlendOp;
        m_Key.srcAlphaBlendFactor = srcAlphaBlendFactor;
        m_Key.dstAlphaBlendFactor = dstAlphaBlendFactor;
        m_Key.alphaBlendOp = alphaBlendOp;
        m_Key.colorWriteMask = colorWriteMask;
        return *this;
    }

    VkPipeline PipelineBuilder::Build(
        const std::shared_ptr<VulkanDevice> &device,
        const VkPipelineLayout &layout,
        const std::shared_ptr<PipelineCache> &cache
    ) const {
        return cache->GetOrCreatePipeline(m_Key, device, layout);
    }
}
