#include "vulkan_renderer_with_ui.h"

#include <iostream>

#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "../../../engine/renderer/vulkan/utils.h"
#include "../../../engine/renderer/vulkan/vertex_utils.h"
#include "../../../engine/renderer/vulkan/vulkan_device.h"


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
    if (m_ViewportExtent.height == 0) {
        return 1.0f;
    }
    return static_cast<float>(m_ViewportExtent.width) / static_cast<float>(m_ViewportExtent.height);
}

void Vulkan::RendererWithUi::Cleanup() {
    CleanupViewportResources();
    m_Device->DestroyRenderPass(m_ViewportRenderPass);

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

    m_PickingRequest.pos = {
        static_cast<int32_t>(normX * m_ViewportExtent.width),
        static_cast<int32_t>(normY * m_ViewportExtent.height)
    };
    m_PickingRequest.fence = m_InFlightFences[m_CurrentFrameIndex];
    m_PickingRequest.isPending = true;
    m_PickingRequest.frameSubmitted = m_TotalFramesRendered;
}

bool Vulkan::RendererWithUi::IsPickingRequestPending() const {
    return m_PickingRequest.isPending;
}

void Vulkan::RendererWithUi::ExecutePickingPass(const VkCommandBuffer &cmd) const {
    VkRenderPassBeginInfo pickingPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = m_PickingRenderPass,
        .framebuffer = m_PickingFramebuffer,
        .renderArea = {
            {0, 0},
            m_ViewportExtent
        }
    };

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color.uint32[0] = Entities::NULL_ENTITY;
    clearValues[1].depthStencil = {1.0f, 0};
    pickingPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    pickingPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(cmd, &pickingPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PickingPipeline);

    const VkViewport viewport{
        0.0f,
        0.0f,
        static_cast<float>(m_ViewportExtent.width),
        static_cast<float>(m_ViewportExtent.height),
        0.0f,
        1.0f
    };
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    const VkRect2D scissor{
        {0, 0},
        m_ViewportExtent
    };
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    const VkBuffer vertexBuffers[] = {m_VertexBuffer};
    constexpr VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(cmd, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

    const auto dynamicOffset = static_cast<uint32_t>(m_PaddedUniformBufferSize * m_CurrentFrameIndex);
    vkCmdBindDescriptorSets(
        cmd,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_PipelineLayout,
        0,
        1,
        &m_BindlessDescriptorSet,
        0,
        nullptr
    );
    vkCmdBindDescriptorSets(
        cmd,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_PipelineLayout,
        1,
        1,
        &m_DynamicDescriptorSet,
        1,
        &dynamicOffset
    );

    for (const auto &drawCall: m_DrawQueue) {
        PushData pushData = {
            drawCall.worldMatrix,
            drawCall.textureId,
            drawCall.entityId
        };
        vkCmdPushConstants(
            cmd,
            m_PipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushData),
            &pushData
        );
        const auto info = GetMeshManager()->GetMeshInfo(drawCall.meshId);
        vkCmdDrawIndexed(
            cmd,
            info.indexCount,
            1,
            info.indexOffset,
            info.vertexOffset,
            0
        );
    }

    vkCmdEndRenderPass(cmd);
}

void Vulkan::RendererWithUi::RecordPickingCopy(const VkCommandBuffer &cmd) const {
    const VkBufferImageCopy region{
        .bufferOffset = 0,
        .imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
        .imageOffset = {
            m_PickingRequest.pos.x,
            m_PickingRequest.pos.y,
            0
        },
        .imageExtent = {1, 1, 1}
    };

    vkCmdCopyImageToBuffer(
        cmd,
        m_PickingImage,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        m_PickingRequest.buffer,
        1,
        &region
    );
};

void Vulkan::RendererWithUi::UpdatePickingResult() {
    if (!m_PickingRequest.isPending) return;

    const VkResult result = vkGetFenceStatus(
        m_Device->GetLogicalDevice(),
        m_PickingRequest.fence
    );

    if (result == VK_SUCCESS) {
        const auto allocInfo = m_Device->GetAllocationInfo(m_PickingRequest.allocation);
        if (allocInfo.pMappedData) {
            std::memcpy(&m_LastPickedEntityID, allocInfo.pMappedData, sizeof(uint32_t));
        }

        m_PickingRequest.isPending = false;
    }
}

void Vulkan::RendererWithUi::BuildRenderGraph() {
    UpdatePickingResult();

    if (m_PickingRequest.isPending) {
        m_RenderGraph->AddPass({
            .name = "Picking",
            .execute = [this](const VkCommandBuffer &cmd) { ExecutePickingPass(cmd); },
            .usages = {
                {
                    m_PickingImage,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
                }
            },
        });

        m_RenderGraph->AddPass({
            .name = "PickingCopy",
            .execute = [this](const VkCommandBuffer &cmd) {
                RecordPickingCopy(cmd);
            },
            .usages = {
                {
                    m_PickingImage,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    VK_ACCESS_TRANSFER_READ_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT
                }
            },
        });
    }

    m_RenderGraph->AddPass({
        .name = "MainScene",
        .execute = [this](const VkCommandBuffer &cmd) {
            RenderScene(
                cmd,
                m_ViewportFramebuffer,
                m_ViewportExtent
            );
        },
        .usages = {
            {
                m_ViewportImage,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
            }
        },
    });

    m_RenderGraph->AddPass({
        .name = "ImGui",
        .execute = [this](const VkCommandBuffer &cmd) {
            RenderUI(cmd);
        },
        .usages = {
            {
                m_ViewportImage,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_ACCESS_SHADER_READ_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
            },
            {
                m_Swapchain->GetImage(m_ImageIndex),
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
            }
        },
    });

    m_RenderGraph->AddPass({
        .name = "FinalPresentTransition",
        .execute = [](const VkCommandBuffer &) {
        },
        .usages = {
            {
                m_Swapchain->GetImage(m_ImageIndex),
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                0,
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
            }
        }
    });
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
    initInfo.ImageCount = m_Swapchain->GetImageCount();
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

void Vulkan::RendererWithUi::CreateViewportRenderPass() {
    m_ViewportRenderPass = CreateGraphicsRenderPass(
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );
}

void Vulkan::RendererWithUi::CreatePickingResources() {
    const auto extent = m_Swapchain->GetExtent();
    Utils::CreateImage(
        m_Device,
        extent.width,
        extent.height,
        VK_FORMAT_R32_UINT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        m_PickingImage,
        m_PickingAllocation,
        "Picking Image"
    );

    m_ResourceTracker->RegisterImage(
        "Picking_Image",
        m_PickingImage,
        VK_FORMAT_R32_UINT,
        VK_IMAGE_LAYOUT_UNDEFINED
    );

    m_PickingImageView = m_Device->CreateImageView(m_PickingImage, VK_FORMAT_R32_UINT, VK_IMAGE_ASPECT_COLOR_BIT);

    const std::array attachments = {
        m_PickingImageView, m_Swapchain->GetDepthImageView()
    };

    const VkFramebufferCreateInfo framebufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = m_PickingRenderPass,
        .attachmentCount = attachments.size(),
        .pAttachments = attachments.data(),
        .width = extent.width,
        .height = extent.height,
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
        m_PickingRequest.buffer = VK_NULL_HANDLE;
    }

    if (m_PickingRequest.fence != VK_NULL_HANDLE) {
        m_Device->DestroyFence(m_PickingRequest.fence);
        m_PickingRequest.fence = VK_NULL_HANDLE;
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
    CreateViewportRenderPass();
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
        m_ViewportExtent = m_Swapchain->GetExtent();
    }

    Utils::CreateImage(
        m_Device,
        m_ViewportExtent.width, m_ViewportExtent.height,
        m_Swapchain->GetFormat(),
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        m_ViewportImage,
        m_ViewportAllocation,
        "Viewport Image"
    );

    m_ResourceTracker->RegisterImage(
        "Viewport_Image",
        m_ViewportImage,
        m_Swapchain->GetFormat(),
        VK_IMAGE_LAYOUT_UNDEFINED
    );

    m_ViewportImageView = m_Device->CreateImageView(
        m_ViewportImage,
        m_Swapchain->GetFormat(),
        VK_IMAGE_ASPECT_COLOR_BIT
    );

    const std::array attachments = {
        m_ViewportImageView,
        m_Swapchain->GetDepthImageView()
    };

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = m_ViewportRenderPass;
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
    using namespace Shaders;
    // TODO: Pre-compile these shaders

    const auto vertexPath = "../src/editor/renderer/shaders/picking/picking.vert";
    const auto fragmentPath = "../src/editor/renderer/shaders/picking/picking.frag";

    const auto vertexShaderModule = m_ShaderModuleCache->GetOrCreateShaderModule(
        vertexPath,
        ShaderType::Vertex
    );
    const auto fragmentShaderModule = m_ShaderModuleCache->GetOrCreateShaderModule(
        fragmentPath,
        ShaderType::Fragment
    );

    PipelineBuilder builder;

    m_PickingPipeline = builder
            .SetShaders(vertexShaderModule, fragmentShaderModule)
            .SetDepthState(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS)
            .SetTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            .SetRenderPass(m_PickingRenderPass)
            .SetExtent(m_ViewportExtent)
            .SetVertexLayout(POSITION_ONLY)
            .SetRasterizer(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE)
            .SetColorBlending(
                VK_FALSE,
                VK_BLEND_FACTOR_ZERO,
                VK_BLEND_FACTOR_ONE,
                VK_BLEND_OP_ADD,
                VK_BLEND_FACTOR_ZERO,
                VK_BLEND_FACTOR_ONE,
                VK_BLEND_OP_ADD,
                VK_COLOR_COMPONENT_R_BIT
            )
            .Build(m_Device, m_PipelineLayout, m_PipelineCache);

    m_ShaderModuleCache->DestroyShaderModule(
        fragmentPath,
        ShaderType::Fragment
    );
    m_ShaderModuleCache->DestroyShaderModule(
        vertexPath,
        ShaderType::Vertex
    );
}

void Vulkan::RendererWithUi::RenderUI(const VkCommandBuffer &cmd) const {
    VkRenderPassBeginInfo uiPassInfo{};
    uiPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    uiPassInfo.renderPass = m_MainRenderPass;
    uiPassInfo.framebuffer = m_Swapchain->GetFramebuffer(m_ImageIndex);
    uiPassInfo.renderArea.extent = m_Swapchain->GetExtent();
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
        m_ViewportResized = true;
    }

    if (m_ViewportResized) {
        m_ViewportExtent.width = width;
        m_ViewportExtent.height = height;

        WaitIdle();

        m_Device->DestroyRenderPass(m_ViewportRenderPass);
        CleanupViewportResources();

        CreateViewportRenderPass();
        CreateViewportResources();

        m_ViewportResized = false;
    }
}

Entities::EntityID Vulkan::RendererWithUi::GetLastPickedID() const {
    return m_LastPickedEntityID;
}

std::shared_ptr<Window> Vulkan::RendererWithUi::GetWindow() {
    return m_Window;
}

void Vulkan::RendererWithUi::AddResizeCallbacks() {
    Renderer::AddResizeCallbacks();
    m_Window->AddResizeCallback([this](Extent) {
        m_ViewportResized = true;
    });
}

VkDescriptorSet Vulkan::RendererWithUi::GetViewportDescriptorSet() const {
    return m_ViewportDescriptorSet;
}
