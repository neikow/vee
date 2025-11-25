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
    m_CoreRenderer->Initialize(width, height, appName, version);
    InitImgui();
    m_CoreRenderer->ToggleRenderMode(RenderMode::EDITOR);
}

void Vulkan::RendererWithUi::Draw() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    m_CoreRenderer->Draw();
}

void Vulkan::RendererWithUi::SubmitUIDrawData(ImDrawData *drawData) {
    m_DrawData = drawData;
}

void Vulkan::RendererWithUi::UpdateCameraMatrix(const glm::mat4x4 &viewMatrix, const glm::mat4x4 &projectionMatrix) {
    m_CoreRenderer->UpdateCameraMatrix(viewMatrix, projectionMatrix);
}

void Vulkan::RendererWithUi::SubmitDrawCall(
    const glm::mat4x4 &worldMatrix,
    std::uint32_t const meshId,
    const std::uint32_t textureId
) {
    m_CoreRenderer->SubmitDrawCall(
        worldMatrix, meshId, textureId
    );
}

void Vulkan::RendererWithUi::Cleanup() {
    // This is a hack as the current architecture does not provide a way to destroy the viewport resources
    // before destroying ImGui resources.
    m_CoreRenderer->ToggleRenderMode(RenderMode::ENGINE);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    vkDestroyDescriptorPool(
        m_CoreRenderer->GetDevice(),
        m_ImguiDescriptorPool,
        nullptr
    );

    m_CoreRenderer->Cleanup();

    ImGui::DestroyContext();
}

bool Vulkan::RendererWithUi::ShouldClose() {
    return m_CoreRenderer->ShouldClose();
}

void Vulkan::RendererWithUi::WaitIdle() {
    m_CoreRenderer->WaitIdle();
}

float Vulkan::RendererWithUi::GetAspectRatio() {
    return m_CoreRenderer->GetAspectRatio();
}

void Vulkan::RendererWithUi::UpdateViewportSize(const uint32_t width, const uint32_t height) const {
    m_CoreRenderer->UpdateViewportSize(width, height);
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

    if (vkCreateDescriptorPool(m_CoreRenderer->GetDevice(), &poolInfo, nullptr, &m_ImguiDescriptorPool) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create ImGui descriptor pool!");
    }

    ImGui_ImplGlfw_InitForVulkan(m_CoreRenderer->m_Window, true);

    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance = m_CoreRenderer->m_Instance;
    initInfo.PhysicalDevice = m_CoreRenderer->m_PhysicalDevice;
    initInfo.Allocator = nullptr;
    initInfo.Device = m_CoreRenderer->m_Device;
    initInfo.Queue = m_CoreRenderer->m_GraphicsQueue;
    initInfo.QueueFamily = m_CoreRenderer->FindQueueFamilies(m_CoreRenderer->m_PhysicalDevice).graphicsFamily.value();
    initInfo.DescriptorPool = m_ImguiDescriptorPool;
    initInfo.ImageCount = m_CoreRenderer->m_SwapChainImages.size();
    initInfo.MinImageCount = MAX_FRAMES_IN_FLIGHT;
    initInfo.PipelineCache = VK_NULL_HANDLE;
    initInfo.PipelineInfoMain.RenderPass = m_CoreRenderer->m_RenderPass;
    initInfo.PipelineInfoMain.Subpass = 0;
    initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    initInfo.ApiVersion = VK_API_VERSION_1_3;

    if (!ImGui_ImplVulkan_Init(&initInfo)) {
        throw std::runtime_error("failed to initialize ImGui Vulkan backend!");
    }
}
