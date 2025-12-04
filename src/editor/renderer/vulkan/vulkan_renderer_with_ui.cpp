#include "vulkan_renderer_with_ui.h"

#include <iostream>

#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"


void Vulkan::RendererWithUi::Initialize(
    const int width,
    const int height,
    const std::string &appName,
    const uint32_t version
) {
    Renderer::Initialize(width, height, appName, version);
    InitImgui();
    ToggleRenderMode(RenderMode::EDITOR);
}

void Vulkan::RendererWithUi::Draw() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    Renderer::Draw();
}

void Vulkan::RendererWithUi::SubmitUIDrawData(ImDrawData *drawData) {
    m_DrawData = drawData;
}

void Vulkan::RendererWithUi::Cleanup() {
    // This is a hack as the current architecture does not provide a way to destroy the viewport resources
    // before destroying ImGui resources.
    ToggleRenderMode(RenderMode::ENGINE);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    vkDestroyDescriptorPool(
        GetDevice(),
        m_ImguiDescriptorPool,
        nullptr
    );

    CleanupPickingResources();

    Renderer::Cleanup();

    ImGui::DestroyContext();
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
    poolInfo.poolSizeCount = (uint32_t) IM_ARRAYSIZE(pool_sizes);
    poolInfo.pPoolSizes = pool_sizes;

    if (vkCreateDescriptorPool(GetDevice(), &poolInfo, nullptr, &m_ImguiDescriptorPool) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create ImGui descriptor pool!");
    }

    ImGui_ImplGlfw_InitForVulkan(m_Window, true);

    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance = m_Instance;
    initInfo.PhysicalDevice = m_PhysicalDevice;
    initInfo.Allocator = nullptr;
    initInfo.Device = m_Device;
    initInfo.Queue = m_GraphicsQueue;
    initInfo.QueueFamily = FindQueueFamilies(m_PhysicalDevice).graphicsFamily.value();
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
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_PickingImage,
        m_PickingMemory,
        {}
    );

    m_PickingImageView = CreateImageView(m_PickingImage, VK_FORMAT_R32_UINT, VK_IMAGE_ASPECT_COLOR_BIT);

    EnqueueCleanupTask([this] {
        vkDestroyImageView(m_Device, m_PickingImageView, nullptr);
    });

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

    if (vkCreateFramebuffer(m_Device, &framebufferCreateInfo, nullptr, &m_PickingFramebuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create picking framebuffer!");
    }
}

void Vulkan::RendererWithUi::CleanupPickingResources() const {
    vkDestroyImageView(m_Device, m_PickingImageView, nullptr);
    vkDestroyImage(m_Device, m_PickingImage, nullptr);
    vkFreeMemory(m_Device, m_PickingMemory, nullptr);
    vkDestroyFramebuffer(m_Device, m_PickingFramebuffer, nullptr);
}

void Vulkan::RendererWithUi::CreateGraphicsResources() {
    Renderer::CreateGraphicsResources();

    CreatePickingResources();
    CreatePickingPipeline();
    CreatePickingPipeline();
}

void Vulkan::RendererWithUi::CreatePickingRenderPass() {
}

void Vulkan::RendererWithUi::CreatePickingPipeline() {
}
