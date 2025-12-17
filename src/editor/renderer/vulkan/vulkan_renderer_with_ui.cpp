#include "vulkan_renderer_with_ui.h"

#include <iostream>

#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "../../../engine/renderer/vulkan/utils.h"
#include "../../../engine/renderer/vulkan/vertex_utils.h"
#include "../../../engine/renderer/vulkan/vulkan_device.h"
#include "../../../engine/shaders/compile.h"


void Vulkan::RendererWithUi::Initialize(
    const std::string &appName,
    const uint32_t version
) {
    Renderer::Initialize(appName, version);
    InitImgui();
    CreateViewportResources();
}

Vulkan::RendererWithUi::RendererWithUi(const std::shared_ptr<Window> &window) : Renderer(window), m_PickingRequest() {
}

float Vulkan::RendererWithUi::GetAspectRatio() {
    return static_cast<float>(m_ViewportExtent.width) / static_cast<float>(m_ViewportExtent.height);
}

void Vulkan::RendererWithUi::Cleanup() {
    CleanupViewportResources();

    CleanupPickingResources();
    m_Device->DestroyRenderPass(m_PickingRenderPass);
    m_Device->DestroyPipeline(m_PickingPipeline);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    m_Device->DestroyDescriptorPool(m_ImguiDescriptorPool);

    Renderer::Cleanup();

    ImGui::DestroyContext();
}

void Vulkan::RendererWithUi::RequestEntityIDAt(const double normX, const double normY) {
    if (m_PickingRequest.isPending) return;
    if (normX < 0.0 || normX > 1.0 || normY < 0.0 || normY > 1.0) return;

    if (m_PickingRequest.buffer == VK_NULL_HANDLE) {
        CreateBuffer(
            sizeof(uint32_t),
            VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VMA_MEMORY_USAGE_AUTO,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            m_PickingRequest.buffer, m_PickingRequest.allocation,
            "Async Picking Buffer"
        );
    }

    const VkCommandBuffer cmd = BeginSingleTimeCommands(m_Device->GetGraphicsCommandPool());

    VkRenderPassBeginInfo pickingPassInfo{};
    pickingPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    pickingPassInfo.renderPass = m_PickingRenderPass;
    pickingPassInfo.framebuffer = m_PickingFramebuffer;
    pickingPassInfo.renderArea.offset = {0, 0};
    pickingPassInfo.renderArea.extent = m_ViewportExtent;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color.uint32[0] = Entities::NULL_ENTITY;
    clearValues[1].depthStencil = {1.0f, 0};
    pickingPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    pickingPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(cmd, &pickingPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PickingPipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_ViewportExtent.width);
    viewport.height = static_cast<float>(m_ViewportExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_ViewportExtent;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    const VkBuffer vertexBuffers[] = {m_VertexBuffer};
    constexpr VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(cmd, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(
        cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout,
        0, 1, &m_DescriptorSets[m_CurrentFrameIndex], 0, nullptr
    );

    for (const auto &drawCall: m_DrawQueue) {
        PushData pushData = {
            drawCall.worldMatrix,
            drawCall.textureId,
            drawCall.entityId,
        };

        vkCmdPushConstants(
            cmd, m_PipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0, sizeof(PushData), &pushData
        );

        const auto info = GetMeshManager()->GetMeshInfo(drawCall.meshId);
        vkCmdDrawIndexed(cmd, info.indexCount, 1, info.indexOffset, info.vertexOffset, 0);
    }

    vkCmdEndRenderPass(cmd);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {
        static_cast<int32_t>(normX * m_ViewportExtent.width),
        static_cast<int32_t>(normY * m_ViewportExtent.height),
        0
    };
    region.imageExtent = {1, 1, 1};

    vkCmdCopyImageToBuffer(
        cmd,
        m_PickingImage,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        m_PickingRequest.buffer,
        1,
        &region
    );

    // We don't need to wait for the result immediately so we don't call EndSingleTimeCommands();
    vkEndCommandBuffer(cmd);

    VkSubmitInfo submitInfo{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;

    vkQueueSubmit(
        m_Device->GetGraphicsQueue(),
        1,
        &submitInfo,
        VK_NULL_HANDLE
    );

    m_PickingRequest.isPending = true;
    m_PickingRequest.frameSubmitted = m_TotalFramesRendered;
}

bool Vulkan::RendererWithUi::IsPickingRequestPending() const {
    return m_PickingRequest.isPending;
}

void Vulkan::RendererWithUi::UpdatePickingResult() {
    if (!m_PickingRequest.isPending) return;

    if (m_TotalFramesRendered < m_PickingRequest.frameSubmitted + MAX_FRAMES_IN_FLIGHT) {
        return;
    }

    const auto allocInfo = m_Device->GetAllocationInfo(m_PickingAllocation);
    if (allocInfo.pMappedData) {
        std::memcpy(&m_LastPickedEntityID, allocInfo.pMappedData, sizeof(uint32_t));
    }

    m_PickingRequest.isPending = false;
}

void Vulkan::RendererWithUi::Draw() {
    UpdatePickingResult();
    Renderer::Draw();
}

void Vulkan::RendererWithUi::InitImgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    const VkDescriptorPoolSize pool_sizes[] = {
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
    };
    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    poolInfo.poolSizeCount = static_cast<uint32_t>(IM_ARRAYSIZE(pool_sizes));
    poolInfo.pPoolSizes = pool_sizes;

    m_ImguiDescriptorPool = m_Device->CreateDescriptorPool(poolInfo);

    ImGui_ImplGlfw_InitForVulkan(m_Window->GetWindow(), true);

    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance = m_Device->GetInstance();
    initInfo.PhysicalDevice = m_Device->GetPhysicalDevice();
    initInfo.Allocator = nullptr;
    initInfo.Device = m_Device->GetLogicalDevice();
    initInfo.Queue = m_Device->GetGraphicsQueue();
    initInfo.QueueFamily = m_Device->FindQueueFamilies(m_Device->GetPhysicalDevice()).graphicsFamily.value();
    initInfo.DescriptorPool = m_ImguiDescriptorPool;
    initInfo.ImageCount = m_SwapChainImages.size();
    initInfo.MinImageCount = MAX_FRAMES_IN_FLIGHT;
    initInfo.PipelineCache = VK_NULL_HANDLE;
    initInfo.PipelineInfoMain.RenderPass = m_MainRenderPass;
    initInfo.PipelineInfoMain.Subpass = 0;
    initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    initInfo.ApiVersion = VK_API_VERSION_1_3;

    if (!ImGui_ImplVulkan_Init(&initInfo)) {
        throw std::runtime_error("failed to initialize ImGui Vulkan backend!");
    }
}

void Vulkan::RendererWithUi::CreatePickingResources() {
    CreateImage(
        m_SwapChainExtent.width,
        m_SwapChainExtent.height,
        VK_FORMAT_R32_UINT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        m_PickingImage,
        m_PickingAllocation,
        "Picking Image"
    );

    m_PickingImageView = CreateImageView(m_PickingImage, VK_FORMAT_R32_UINT, VK_IMAGE_ASPECT_COLOR_BIT);

    const std::array attachments = {
        m_PickingImageView,
        m_DepthImageView
    };

    const VkFramebufferCreateInfo framebufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = m_PickingRenderPass,
        .attachmentCount = attachments.size(),
        .pAttachments = attachments.data(),
        .width = m_SwapChainExtent.width,
        .height = m_SwapChainExtent.height,
        .layers = 1
    };

    m_PickingFramebuffer = m_Device->CreateFramebuffer(framebufferCreateInfo);
}

void Vulkan::RendererWithUi::CleanupPickingResources() {
    if (m_PickingImageView) {
        m_Device->DestroyImageView(m_PickingImageView);
        m_PickingImageView = nullptr;
    }
    if (m_PickingRequest.buffer) {
        m_Device->DestroyBuffer(m_PickingRequest.buffer, m_PickingRequest.allocation);
    }
    m_Device->DestroyImage(m_PickingImage, m_PickingAllocation);
    m_Device->DestroyFramebuffer(m_PickingFramebuffer);
}

void Vulkan::RendererWithUi::PrepareForRendering() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
}

void Vulkan::RendererWithUi::CreateGraphicsResources() {
    Renderer::CreateGraphicsResources();
    CreatePickingRenderPass();
    CreatePickingPipeline();
    CreatePickingResources();
}

void Vulkan::RendererWithUi::CreatePickingRenderPass() {
    constexpr VkAttachmentDescription colorAttachment{
        .format = VK_FORMAT_R32_UINT,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    };

    constexpr VkAttachmentReference colorAttachmentRef{
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    const VkAttachmentDescription depthAttachment{
        .format = Utils::FindDepthFormat(m_Device->GetPhysicalDevice()),
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    constexpr VkAttachmentReference depthAttachmentRef{
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    const VkSubpassDescription subpass{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
        .pDepthStencilAttachment = &depthAttachmentRef
    };

    std::array attachments = {colorAttachment, depthAttachment};
    const VkRenderPassCreateInfo renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = attachments.size(),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 0,
        .pDependencies = nullptr
    };

    m_PickingRenderPass = m_Device->CreateRenderPass(renderPassInfo);
}

void Vulkan::RendererWithUi::CreateViewportResources() {
    if (m_ViewportExtent.width == 0 || m_ViewportExtent.height == 0) {
        m_ViewportExtent = m_SwapChainExtent;
    }

    CreateImage(
        m_ViewportExtent.width, m_ViewportExtent.height,
        m_SwapChainImageFormat, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
        m_ViewportImage, m_ViewportAllocation,
        "Viewport Image"
    );

    m_ViewportImageView = CreateImageView(m_ViewportImage, m_SwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

    const std::array attachments = {
        m_ViewportImageView,
        m_DepthImageView
    };

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = m_MainRenderPass;
    framebufferInfo.attachmentCount = attachments.size();
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = m_ViewportExtent.width;
    framebufferInfo.height = m_ViewportExtent.height;
    framebufferInfo.layers = 1;

    m_ViewportFramebuffer = m_Device->CreateFramebuffer(framebufferInfo);

    m_ViewportDescriptorSet = ImGui_ImplVulkan_AddTexture(
        m_TextureSampler,
        m_ViewportImageView,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );
}

void Vulkan::RendererWithUi::CleanupViewportResources() const {
    WaitIdle();

    m_Device->DestroyImageView(m_ViewportImageView);
    m_Device->DestroyImage(m_ViewportImage, m_ViewportAllocation);
    m_Device->DestroyFramebuffer(m_ViewportFramebuffer);

    ImGui_ImplVulkan_RemoveTexture(m_ViewportDescriptorSet);
}

void Vulkan::RendererWithUi::CreatePickingPipeline() {
    // TODO: Pre-compile these shaders
    const auto vertShaderCode = Shaders::CompileFromFile(
        "../src/editor/renderer/shaders/picking/picking.vert",
        shaderc_vertex_shader
    );
    const auto fragShaderCode = Shaders::CompileFromFile(
        "../src/editor/renderer/shaders/picking/picking.frag",
        shaderc_fragment_shader
    );

    const auto vertShaderModule = CreateShaderModule(vertShaderCode);
    const auto fragShaderModule = CreateShaderModule(fragShaderCode);

    const VkPipelineShaderStageCreateInfo vertShaderStageInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertShaderModule,
        .pName = "main",
    };

    const VkPipelineShaderStageCreateInfo fragShaderStageInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragShaderModule,
        .pName = "main",
    };

    constexpr VkPipelineDepthStencilStateCreateInfo depthStencilInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
        .front = {},
        .back = {},
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 1.0f,
    };

    const std::array shaderStages = {vertShaderStageInfo, fragShaderStageInfo};

    std::vector dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data(),
    };

    auto bindingDescription = VertexUtils::GetBindingDescription();
    auto attributeDescriptions = VertexUtils::GetPickingAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = attributeDescriptions.size(),
        .pVertexAttributeDescriptions = attributeDescriptions.data(),
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    VkViewport viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(m_ViewportExtent.width),
        .height = static_cast<float>(m_ViewportExtent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    VkRect2D scissor{
        .offset = {0, 0},
        .extent = m_ViewportExtent,
    };

    VkPipelineViewportStateCreateInfo viewportState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };

    VkPipelineRasterizationStateCreateInfo rasterizer{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f,
    };

    VkPipelineMultisampleStateCreateInfo multisampling{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment{
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT,
    };

    VkPipelineColorBlendStateCreateInfo colorBlendState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
    };

    VkGraphicsPipelineCreateInfo pipelineCreateInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = shaderStages.size(),
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = &depthStencilInfo,
        .pColorBlendState = &colorBlendState,
        .pDynamicState = &dynamicState,
        .layout = m_PipelineLayout,
        .renderPass = m_PickingRenderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1,
    };

    m_PickingPipeline = m_Device->CreatePipeline(pipelineCreateInfo);

    m_Device->DestroyShaderModule(fragShaderModule);
    m_Device->DestroyShaderModule(vertShaderModule);
}

void Vulkan::RendererWithUi::RenderToScreen(const VkCommandBuffer &cmd) {
    RenderScene(cmd, m_ViewportFramebuffer, m_ViewportExtent);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_ViewportImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    VkRenderPassBeginInfo uiPassInfo{};
    uiPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    uiPassInfo.renderPass = m_MainRenderPass;
    uiPassInfo.framebuffer = m_SwapChainFramebuffers[m_ImageIndex];
    uiPassInfo.renderArea.extent = m_SwapChainExtent;
    uiPassInfo.clearValueCount = 0;
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    uiPassInfo.clearValueCount = 2;
    uiPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(cmd, &uiPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    if (ImDrawData *drawData = ImGui::GetDrawData()) {
        ImGui_ImplVulkan_RenderDrawData(drawData, cmd);
    }

    vkCmdEndRenderPass(cmd);
}

void Vulkan::RendererWithUi::UpdateViewportSize(const uint32_t width, const uint32_t height) {
    if (width != m_ViewportExtent.width || height != m_ViewportExtent.height) {
        m_ViewportExtent.width = width;
        m_ViewportExtent.height = height;

        WaitIdle();

        CleanupViewportResources();
        CreateViewportResources();
    }
}

Entities::EntityID Vulkan::RendererWithUi::GetLastPickedID() const {
    return m_LastPickedEntityID;
}

std::shared_ptr<Window> Vulkan::RendererWithUi::GetWindow() {
    return m_Window;
}

VkDescriptorSet Vulkan::RendererWithUi::GetViewportDescriptorSet() const {
    return m_ViewportDescriptorSet;
}
