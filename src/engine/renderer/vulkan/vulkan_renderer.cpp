#include "vulkan_renderer.h"

#include <map>
#include <GLFW/glfw3.h>
#include <shaderc/shaderc.h>

#include "imgui_impl_vulkan.h"
#include "types.h"
#include "utils.h"
#include "vertex_utils.h"
#include "../../engine.h"
#include "../../shaders/compile.h"
#include "../../utils/renderer/deletion_queue.h"
#include "vulkan_device.h"
#include "vk_mem_alloc.h"

static constexpr uint8_t DEFAULT_PURPLE_PIXEL[4] = {255, 0, 255, 255};

constexpr int MAX_TEXTURES = 2048;

namespace Vulkan {
    std::shared_ptr<VulkanDevice> Renderer::GetDevice() const {
        return m_Device;
    }

    void Renderer::FramebufferResizeCallback(GLFWwindow *window, int, int) {
        const auto renderer = static_cast<Renderer *>(glfwGetWindowUserPointer(window));
        renderer->m_FramebufferResized = true;
    }

    void Renderer::UpdateTextureDescriptor(const TextureId textureId) {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = GetTextureManager()->GetImageView(textureId);
        imageInfo.sampler = m_TextureSampler;

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstBinding = 2;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;

        descriptorWrite.dstArrayElement = textureId;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            descriptorWrite.dstSet = m_DescriptorSets[i];
            vkUpdateDescriptorSets(m_Device->GetLogicalDevice(), 1, &descriptorWrite, 0, nullptr);
        }
    }

    void Renderer::UpdateGeometryBuffers() {
        WaitIdle();

        const VkBuffer oldVertexBuffer = m_VertexBuffer;
        const VmaAllocation oldVertexAlloc = m_VertexAllocation;
        const VkBuffer oldIndexBuffer = m_IndexBuffer;
        const VmaAllocation oldIndexAlloc = m_IndexAllocation;

        m_VertexBuffer = VK_NULL_HANDLE;
        m_IndexBuffer = VK_NULL_HANDLE;

        CreateVertexBuffer();
        CreateIndexBuffer();

        vmaDestroyBuffer(m_Device->GetAllocator(), oldVertexBuffer, oldVertexAlloc);
        vmaDestroyBuffer(m_Device->GetAllocator(), oldIndexBuffer, oldIndexAlloc);
    }

    void Renderer::DumpVmaStats() const {
        if (m_Device->GetAllocator() == VK_NULL_HANDLE) return;

        char *statsString = nullptr;
        vmaBuildStatsString(m_Device->GetAllocator(), &statsString, VK_TRUE);

        if (statsString) {
            std::cout << "--- VMA LEAK REPORT ---" << std::endl;
            std::cout << statsString << std::endl;
            std::cout << "-----------------------" << std::endl;

            vmaFreeStatsString(m_Device->GetAllocator(), statsString);
        }
    }

    MemoryUsage Renderer::GetMemoryUsage() {
        MemoryUsage usage{};

        if (m_Device->GetAllocator() == VK_NULL_HANDLE) return usage;

        VmaBudget budgets[VK_MAX_MEMORY_HEAPS];
        vmaGetHeapBudgets(m_Device->GetAllocator(), budgets);

        usage.usedMemoryMB += budgets[0].usage / (1024.0f * 1024.0f);
        usage.availableMemoryMB += budgets[0].budget / (1024.0f * 1024.0f);

        return usage;
    }

    float Renderer::GetAspectRatio() {
        return static_cast<float>(m_SwapChainExtent.width) / static_cast<float>(m_SwapChainExtent.height);
    }

    void Renderer::InitVulkan() {
        CreatePipelinesAndShaders();
        CreateGraphicsResources();
        CreateBuffers();
        CreateDescriptorAndSyncObjects();
    }


    void Renderer::CreateGraphicsResources() {
        CreateSwapChain();
        CreateImageViews();
        CreateDepthResources();
        CreateMissingTexture();
        CreateFramebuffers();
    }

    void Renderer::SetSwapChainImageFormat() {
        const SwapChainSupportDetails swapChainSupport = m_Device->QuerySwapChainSupport(
            m_Device->GetPhysicalDevice()
        );

        const VkSurfaceFormatKHR surfaceFormat = Utils::ChooseSwapSurfaceFormat(swapChainSupport.formats);
        m_SwapChainImageFormat = surfaceFormat.format;
    }

    void Renderer::CreatePipelinesAndShaders() {
        SetSwapChainImageFormat();
        CreateMainRenderPass();
        CreateDescriptorSetLayout();
        CreateGraphicsPipeline();
    }

    void Renderer::CreateBuffers() {
        CreateVertexBuffer();
        CreateIndexBuffer();
        CreateUniformBuffers();
    }

    void Renderer::CreateDescriptorAndSyncObjects() {
        CreateTextureSampler();
        CreateDescriptorPool();
        CreateDescriptorSets();
        CreateCommandBuffers();
        CreateSyncObjects();
    }

    void Renderer::CreateSyncObjects() {
        m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_RenderFinishedSemaphores.resize(m_SwapChainImages.size());
        m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(
                    m_Device->GetLogicalDevice(),
                    &semaphoreInfo,
                    nullptr,
                    &m_ImageAvailableSemaphores[i]
                ) != VK_SUCCESS
                || vkCreateFence(
                    m_Device->GetLogicalDevice(),
                    &fenceInfo,
                    nullptr,
                    &m_InFlightFences[i]
                ) != VK_SUCCESS
            ) {
                throw std::runtime_error("failed to create per-frame sync objects!");
            }
        }

        for (auto &renderFinishedSemaphore: m_RenderFinishedSemaphores) {
            if (vkCreateSemaphore(
                    m_Device->GetLogicalDevice(),
                    &semaphoreInfo,
                    nullptr,
                    &renderFinishedSemaphore
                ) != VK_SUCCESS
            ) {
                throw std::runtime_error("failed to create per-image renderFinished semaphore!");
            }
        }
    }

    void Renderer::CreateCommandBuffers() {
        m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_Device->GetGraphicsCommandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = m_CommandBuffers.size();

        if (vkAllocateCommandBuffers(
                m_Device->GetLogicalDevice(),
                &allocInfo,
                m_CommandBuffers.data()
            ) != VK_SUCCESS
        ) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void Renderer::CreateMissingTexture() {
        constexpr VkDeviceSize imageSize = 4;
        VkBuffer stagingBuffer;
        VmaAllocation stagingAlloc;

        CreateBuffer(
            imageSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            stagingBuffer, stagingAlloc,
            "Missing Texture Staging Buffer"
        );

        void *data;
        m_Device->MapMemory(stagingAlloc, &data);
        std::memcpy(data, DEFAULT_PURPLE_PIXEL, imageSize);
        m_Device->UnmapMemory(stagingAlloc);

        CreateImage(
            1, 1, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            m_DefaultTextureImage,
            m_DefaultTextureAllocation,
            "Missing Texture"
        );

        TransitionImageLayout(
            m_DefaultTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        );
        CopyBufferToImage(
            stagingBuffer, m_DefaultTextureImage, 1, 1
        );
        TransitionImageLayout(
            m_DefaultTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        m_DefaultTextureImageView = CreateImageView(
            m_DefaultTextureImage,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_ASPECT_COLOR_BIT
        );

        m_ResourceDeletionQueue->Enqueue({
            [this, stagingBuffer, stagingAlloc] {
                vmaDestroyBuffer(
                    m_Device->GetAllocator(),
                    stagingBuffer,
                    stagingAlloc
                );
            },
            m_TotalFramesRendered
        });
    }

    void Renderer::CreateDescriptorSets() {
        if (!GetTextureManager()) {
            throw std::runtime_error("CreateDescriptorSets called but m_TextureManager is null");
        }

        const std::vector layouts(MAX_FRAMES_IN_FLIGHT, m_DescriptorSetLayout);

        const std::vector<uint32_t> variableDescriptorCounts(MAX_FRAMES_IN_FLIGHT, MAX_TEXTURES);

        VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
        variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
        variableCountInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        variableCountInfo.pDescriptorCounts = variableDescriptorCounts.data();

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();
        allocInfo.pNext = &variableCountInfo;

        m_DescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(
                m_Device->GetLogicalDevice(),
                &allocInfo,
                m_DescriptorSets.data()
            ) != VK_SUCCESS
        ) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_UniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            std::vector<VkDescriptorImageInfo> textureInfos(MAX_TEXTURES);

            for (uint32_t j = 0; j < MAX_TEXTURES; ++j) {
                textureInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                if (const auto textureView = GetTextureManager()->GetImageView(j); textureView != VK_NULL_HANDLE)
                    textureInfos[j].imageView = textureView;
                else
                    textureInfos[j].imageView = m_DefaultTextureImageView;
            }

            std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = m_DescriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            VkDescriptorImageInfo samplerInfo{};
            samplerInfo.sampler = m_TextureSampler;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = m_DescriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &samplerInfo;

            descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[2].dstSet = m_DescriptorSets[i];
            descriptorWrites[2].dstBinding = 2;
            descriptorWrites[2].dstArrayElement = 0;
            descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            descriptorWrites[2].descriptorCount = MAX_TEXTURES;
            descriptorWrites[2].pImageInfo = textureInfos.data();

            vkUpdateDescriptorSets(
                m_Device->GetLogicalDevice(),
                descriptorWrites.size(),
                descriptorWrites.data(),
                0,
                nullptr
            );
        }
    }

    void Renderer::CreateDescriptorPool() {
        std::array<VkDescriptorPoolSize, 3> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        poolSizes[2].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        poolSizes[2].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * MAX_TEXTURES);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

        if (vkCreateDescriptorPool(
                m_Device->GetLogicalDevice(),
                &poolInfo,
                nullptr,
                &m_DescriptorPool
            ) != VK_SUCCESS
        ) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    void Renderer::CreateUniformBuffers() {
        m_UniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        m_UniformBuffersAllocations.resize(MAX_FRAMES_IN_FLIGHT);
        m_UniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            constexpr VkDeviceSize bufferSize = sizeof(UniformBufferObject);

            VkBufferCreateInfo bufferInfo{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
            bufferInfo.size = bufferSize;
            bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VmaAllocationCreateInfo allocInfo = {};
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
            allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                              VMA_ALLOCATION_CREATE_MAPPED_BIT;

            VmaAllocationInfo resultInfo;
            if (vmaCreateBuffer(m_Device->GetAllocator(), &bufferInfo, &allocInfo,
                                &m_UniformBuffers[i], &m_UniformBuffersAllocations[i],
                                &resultInfo) != VK_SUCCESS) {
                throw std::runtime_error("failed to create mapped uniform buffer!");
            }

            m_UniformBuffersMapped[i] = resultInfo.pMappedData;
        }
    }

    void Renderer::CreateIndexBuffer() {
        const auto indices = GetMeshManager()->GetMeshIndicesArray();
        const VkDeviceSize requiredSize = sizeof(indices[0]) * indices.size();

        if (m_IndexBuffer == VK_NULL_HANDLE || requiredSize > m_CurrentIndexBufferSize) {
            if (m_IndexBuffer != VK_NULL_HANDLE) {
                vmaDestroyBuffer(m_Device->GetAllocator(), m_IndexBuffer, m_IndexAllocation);
            }

            m_CurrentIndexBufferSize = std::max(MAX_GEOMETRY_BUFFER_SIZE, requiredSize);

            CreateBuffer(
                m_CurrentIndexBufferSize,
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
                0,
                m_IndexBuffer, m_IndexAllocation,
                "Global Index Buffer"
            );
        }

        UploadToBuffer(m_IndexBuffer, indices, requiredSize);
    }

    template<typename T>
    void Renderer::UploadToBuffer(
        const VkBuffer &targetBuffer,
        const std::vector<T> &data,
        const size_t size
    ) {
        if (data.empty()) return;

        VkBuffer stagingBuffer;
        VmaAllocation stagingAllocation;

        CreateBuffer(
            size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            stagingBuffer, stagingAllocation,
            "Staging Vertex Buffer"
        );

        void *stagingData;
        vmaMapMemory(m_Device->GetAllocator(), stagingAllocation, &stagingData);
        std::memcpy(stagingData, data.data(), size);
        vmaUnmapMemory(m_Device->GetAllocator(), stagingAllocation);

        CopyBuffer(stagingBuffer, targetBuffer, size);

        m_ResourceDeletionQueue->Enqueue({
            [this, stagingBuffer, stagingAllocation] {
                vmaDestroyBuffer(m_Device->GetAllocator(), stagingBuffer, stagingAllocation);
            },
            m_TotalFramesRendered
        });
    }

    void Renderer::CreateVertexBuffer() {
        const auto vertices = GetMeshManager()->GetMeshVerticesArray();
        const VkDeviceSize requiredSize = sizeof(vertices[0]) * vertices.size();

        if (m_VertexBuffer == VK_NULL_HANDLE || requiredSize > m_CurrentVertexBufferSize) {
            if (m_VertexBuffer != VK_NULL_HANDLE) {
                vmaDestroyBuffer(
                    m_Device->GetAllocator(),
                    m_VertexBuffer,
                    m_VertexAllocation
                );
            }

            m_CurrentVertexBufferSize = std::max(MAX_GEOMETRY_BUFFER_SIZE, requiredSize);

            CreateBuffer(
                m_CurrentVertexBufferSize,
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
                0,
                m_VertexBuffer, m_VertexAllocation,
                "Global Vertex Buffer"
            );
        }

        UploadToBuffer(m_VertexBuffer, vertices, requiredSize);
    }

    void Renderer::CopyBuffer(const VkBuffer srcBuffer, const VkBuffer dstBuffer, const VkDeviceSize size) {
        const VkCommandBuffer commandBuffer = BeginSingleTimeCommands(m_Device->GetTransferCommandPool());

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        EndSingleTimeCommands(commandBuffer, m_Device->GetTransferQueue(), m_Device->GetTransferCommandPool());
    }

    void Renderer::CreateTextureSampler() {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;

        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(
            m_Device->GetPhysicalDevice(),
            &properties
        );

        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        if (
            vkCreateSampler(
                m_Device->GetLogicalDevice(),
                &samplerInfo,
                nullptr,
                &m_TextureSampler
            ) != VK_SUCCESS
        ) {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }

    void Renderer::CreateBuffer(
        const VkDeviceSize size,
        const VkBufferUsageFlags usage,
        const VmaMemoryUsage vmaUsage,
        const VmaAllocationCreateFlags allocationFlags,
        VkBuffer &buffer,
        VmaAllocation &allocation,
        const char *debugName
    ) const {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = vmaUsage;
        allocInfo.flags = allocationFlags;

        if (vmaCreateBuffer(
                m_Device->GetAllocator(),
                &bufferInfo,
                &allocInfo,
                &buffer,
                &allocation,
                nullptr
            ) != VK_SUCCESS
        ) {
            throw std::runtime_error("failed to create VMA buffer!");
        }
        vmaSetAllocationName(m_Device->GetAllocator(), allocation, debugName);
    }

    void Renderer::CopyBufferToImage(
        const VkBuffer &buffer,
        const VkImage &image,
        const uint32_t width,
        const uint32_t height
    ) const {
        const VkCommandBuffer commandBuffer = BeginSingleTimeCommands(
            m_Device->GetTransferCommandPool()
        );

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(
            commandBuffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );

        EndSingleTimeCommands(
            commandBuffer,
            m_Device->GetTransferQueue(),
            m_Device->GetTransferCommandPool()
        );
    }

    void Renderer::CreateFramebuffers() {
        m_SwapChainFramebuffers.resize(m_SwapChainImageViews.size());
        for (size_t i = 0; i < m_SwapChainImageViews.size(); i++) {
            std::array attachments = {
                m_SwapChainImageViews[i],
                m_DepthImageView,
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_MainRenderPass;
            framebufferInfo.attachmentCount = attachments.size();
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = m_SwapChainExtent.width;
            framebufferInfo.height = m_SwapChainExtent.height;
            framebufferInfo.layers = 1;

            if (
                vkCreateFramebuffer(
                    m_Device->GetLogicalDevice(),
                    &framebufferInfo,
                    nullptr,
                    &m_SwapChainFramebuffers[i]
                ) != VK_SUCCESS
            ) {
                throw std::runtime_error("failed to create framebuffer!");
            }

            // Destroyed in CleanupSwapChain()
        }
    }

    void Renderer::CreateImage(
        const uint32_t width,
        const uint32_t height,
        const VkFormat format,
        const VkImageTiling tiling,
        const VkImageUsageFlags usage,
        const VmaMemoryUsage vmaUsage,
        VkImage &image,
        VmaAllocation &allocation,
        const char *debugName
    ) const {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

        if (vkCreateImage(
                m_Device->GetLogicalDevice(),
                &imageInfo,
                nullptr,
                &image
            ) != VK_SUCCESS
        ) {
            throw std::runtime_error("failed to create image!");
        }

        const VmaAllocationCreateInfo allocInfo = {.usage = vmaUsage};

        vmaCreateImage(m_Device->GetAllocator(), &imageInfo, &allocInfo, &image, &allocation, nullptr);
        vmaSetAllocationName(m_Device->GetAllocator(), allocation, debugName);
    }

    VkCommandBuffer Renderer::BeginSingleTimeCommands(
        const VkCommandPool &commandPool
    ) const {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(
            m_Device->GetLogicalDevice(),
            &allocInfo,
            &commandBuffer
        );

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void Renderer::EndSingleTimeCommands(
        const VkCommandBuffer commandBuffer,
        const VkQueue queue,
        const VkCommandPool commandPool
    ) const {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);

        vkFreeCommandBuffers(
            m_Device->GetLogicalDevice(),
            commandPool,
            1,
            &commandBuffer
        );
    }

    void Renderer::TransitionImageLayout(
        const VkImage &image,
        const VkFormat format,
        const VkImageLayout oldLayout,
        const VkImageLayout newLayout
    ) const {
        const VkCommandBuffer commandBuffer = BeginSingleTimeCommands(
            m_Device->GetTransferCommandPool()
        );

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;

        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        barrier.image = image;

        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            if (Utils::HasStencilComponent(format)) {
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        } else {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout ==
                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout ==
                   VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        } else {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        EndSingleTimeCommands(commandBuffer, m_Device->GetTransferQueue(), m_Device->GetTransferCommandPool());
    }

    void Renderer::CreateDepthResources() {
        const VkFormat depthFormat = Utils::FindDepthFormat(m_Device->GetPhysicalDevice());

        CreateImage(
            m_SwapChainExtent.width,
            m_SwapChainExtent.height,
            depthFormat,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            m_DepthImage,
            m_DepthImageAllocation,
            "Depth Image"
        );

        m_DepthImageView = CreateImageView(m_DepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

        TransitionImageLayout(
            m_DepthImage,
            depthFormat,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        );
    }

    void Renderer::CreateGraphicsPipeline() {
        using namespace Shaders;

        const auto vertexPath = "../src/engine/renderer/vulkan/shaders/main.vert";
        const auto fragmentPath = "../src/engine/renderer/vulkan/shaders/main.frag";

        const auto vertShaderModule = m_ShaderModuleCache->GetOrCreateShaderModule(
            vertexPath,
            ShaderType::Vertex
        );
        const auto fragShaderModule = m_ShaderModuleCache->GetOrCreateShaderModule(
            fragmentPath,
            ShaderType::Fragment
        );

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushData);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
        depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilInfo.depthTestEnable = VK_TRUE;
        depthStencilInfo.depthWriteEnable = VK_TRUE;
        depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        depthStencilInfo.minDepthBounds = 0.0f;
        depthStencilInfo.maxDepthBounds = 1.0f;
        depthStencilInfo.stencilTestEnable = VK_FALSE;
        depthStencilInfo.front = {};
        depthStencilInfo.back = {};

        std::array shaderStages = {vertShaderStageInfo, fragShaderStageInfo};

        std::vector dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        auto bindingDescription = VertexUtils::GetBindingDescription();
        auto attributeDescriptions = VertexUtils::GetAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_SwapChainExtent.width);
        viewport.height = static_cast<float>(m_SwapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = m_SwapChainExtent;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f;
        rasterizer.depthBiasClamp = 0.0f;
        rasterizer.depthBiasSlopeFactor = 0.0f;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f;
        multisampling.pSampleMask = nullptr;
        multisampling.alphaToCoverageEnable = VK_FALSE;
        multisampling.alphaToOneEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (
            vkCreatePipelineLayout(
                m_Device->GetLogicalDevice(),
                &pipelineLayoutInfo,
                nullptr,
                &m_PipelineLayout
            ) != VK_SUCCESS
        ) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = shaderStages.size();
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencilInfo;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = m_PipelineLayout;
        pipelineInfo.renderPass = m_MainRenderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        if (vkCreateGraphicsPipelines(
                m_Device->GetLogicalDevice(),
                VK_NULL_HANDLE,
                1,
                &pipelineInfo,
                nullptr,
                &m_GraphicsPipeline
            ) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        m_ShaderModuleCache->DestroyShaderModule(
            vertexPath,
            ShaderType::Vertex
        );
        m_ShaderModuleCache->DestroyShaderModule(
            fragmentPath,
            ShaderType::Fragment
        );
    }

    void Renderer::CreateDescriptorSetLayout() {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutBinding textureLayoutBinding{};
        textureLayoutBinding.binding = 2;
        textureLayoutBinding.descriptorCount = MAX_TEXTURES;
        textureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        textureLayoutBinding.pImmutableSamplers = nullptr;
        textureLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::vector<VkDescriptorBindingFlags> bindingFlags(3); // 3 bindings: UBO, Textures[], Sampler
        bindingFlags[0] = 0; // UBO binding (binding=0) has no special flags
        bindingFlags[1] = 0; // Sampler binding (binding=1)
        bindingFlags[2] =
                VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | // Required for a *dynamically sized* array
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | // Allows some array elements to be unbound/invalid
                VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT; // Specific to sampled images

        VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
        bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
        bindingFlagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
        bindingFlagsInfo.pBindingFlags = bindingFlags.data();

        const std::array bindings = {uboLayoutBinding, samplerLayoutBinding, textureLayoutBinding};
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();
        layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
        layoutInfo.pNext = &bindingFlagsInfo;


        if (vkCreateDescriptorSetLayout(
                m_Device->GetLogicalDevice(),
                &layoutInfo,
                nullptr,
                &m_DescriptorSetLayout
            ) != VK_SUCCESS
        ) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    void Renderer::CreateMainRenderPass() {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = m_SwapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = Utils::FindSupportedFormat(
            m_Device->GetPhysicalDevice(),
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;

        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
                                  | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
                                  | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
                                   | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;


        const std::array attachments = {
            colorAttachment,
            depthAttachment
        };

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(m_Device->GetLogicalDevice(), &renderPassInfo, nullptr, &m_MainRenderPass) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    VkImageView Renderer::CreateImageView(
        const VkImage &image,
        const VkFormat format,
        const VkImageAspectFlags aspectFlags
    ) const {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(m_Device->GetLogicalDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image view!");
        }

        return imageView;
    }

    void Renderer::CreateImageViews() {
        m_SwapChainImageViews.resize(m_SwapChainImages.size());

        for (uint32_t i = 0; i < m_SwapChainImages.size(); i++) {
            m_SwapChainImageViews[i] = CreateImageView(
                m_SwapChainImages[i],
                m_SwapChainImageFormat,
                VK_IMAGE_ASPECT_COLOR_BIT
            );
        }
    }

    void Renderer::CreateSwapChain() {
        const SwapChainSupportDetails swapChainSupport = m_Device->QuerySwapChainSupport(m_Device->GetPhysicalDevice());

        const VkSurfaceFormatKHR surfaceFormat = Utils::ChooseSwapSurfaceFormat(swapChainSupport.formats);
        const VkPresentModeKHR presentMode = Utils::ChooseSwapPresentMode(swapChainSupport.presentModes);
        const VkExtent2D extent = Utils::ChooseSwapExtent(m_Window->GetWindow(), swapChainSupport.capabilities);
        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

        if (
            swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.
            maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_Device->GetSurface();

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = m_SwapChainImageFormat;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        const QueueFamilyIndices indices = m_Device->FindQueueFamilies(m_Device->GetPhysicalDevice());
        const uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(m_Device->GetLogicalDevice(), &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(m_Device->GetLogicalDevice(), m_SwapChain, &imageCount, nullptr);
        m_SwapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(m_Device->GetLogicalDevice(), m_SwapChain, &imageCount, m_SwapChainImages.data());
        m_SwapChainExtent = extent;
    }

    Renderer::Renderer(const std::shared_ptr<Window> &window)
        : m_ResourceDeletionQueue(
              std::make_unique<DeletionQueue>(
                  MAX_FRAMES_IN_FLIGHT
              )
          ),
          m_Window(window),
          m_Device(std::make_shared<VulkanDevice>(window)),
          m_ShaderModuleCache(std::make_shared<ShaderModuleCache>(m_Device)) {
    }

    void Renderer::Initialize(
        const std::string &appName,
        const uint32_t version
    ) {
        m_Device->Initialize(appName, version);
        InitVulkan();
        m_Initialized = true;
        ExecuteInitTasks();
    }

    void Renderer::RecreateSwapChain() {
        int width = 0, height = 0;
        glfwGetFramebufferSize(m_Window->GetWindow(), &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(m_Window->GetWindow(), &width, &height);
            glfwWaitEvents();
        }

        WaitIdle();

        CleanupSwapChain();

        CreateSwapChain();
        CreateImageViews();
        CreateDepthResources();
        CreateFramebuffers();
    }

    void Renderer::RecordDrawQueue(const VkCommandBuffer &commandBuffer) {
        // TODO: sort draw calls by texture to minimize descriptor set changes

        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_PipelineLayout,
            0,
            1,
            &m_DescriptorSets[m_CurrentFrameIndex],
            0,
            nullptr
        );

        for (const auto &drawCall: m_DrawQueue) {
            PushData pushData = {
                drawCall.worldMatrix,
                drawCall.textureId,
                drawCall.entityId,
            };

            vkCmdPushConstants(
                commandBuffer,
                m_PipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(PushData),
                &pushData
            );

            const auto info = GetMeshManager()->GetMeshInfo(drawCall.meshId);

            vkCmdDrawIndexed(
                commandBuffer,
                info.indexCount,
                1,
                info.indexOffset,
                info.vertexOffset,
                0
            );
        }

        m_DrawQueue.clear();
    }

    void Renderer::RenderScene(
        const VkCommandBuffer &commandBuffer,
        const VkFramebuffer &framebuffer,
        const VkExtent2D extent
    ) const {
        if (m_VertexBuffer == VK_NULL_HANDLE || m_IndexBuffer == VK_NULL_HANDLE) {
            return;
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_MainRenderPass;
        renderPassInfo.framebuffer = framebuffer;
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = extent;

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(extent.width);
        viewport.height = static_cast<float>(extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = extent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        const VkBuffer vertexBuffers[] = {m_VertexBuffer};
        constexpr VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(
            commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout,
            0, 1, &m_DescriptorSets[m_CurrentFrameIndex], 0, nullptr
        );

        // 4. Execute Draw Queue
        for (const auto &drawCall: m_DrawQueue) {
            PushData pushData = {
                drawCall.worldMatrix,
                drawCall.textureId,
                drawCall.entityId,
            };

            vkCmdPushConstants(
                commandBuffer, m_PipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0, sizeof(PushData), &pushData
            );

            const auto info = GetMeshManager()->GetMeshInfo(drawCall.meshId);

            vkCmdDrawIndexed(
                commandBuffer,
                info.indexCount,
                1,
                info.indexOffset,
                info.vertexOffset,
                0
            );
        }

        vkCmdEndRenderPass(commandBuffer);
    }

    void Renderer::RenderToScreen(const VkCommandBuffer &cmd) {
        RenderScene(cmd, m_SwapChainFramebuffers[m_ImageIndex], m_SwapChainExtent);
    }

    void Renderer::Draw() {
        m_ResourceDeletionQueue->Flush(m_TotalFramesRendered);

        vkWaitForFences(m_Device->GetLogicalDevice(), 1, &m_InFlightFences[m_CurrentFrameIndex], VK_TRUE, UINT64_MAX);

        VkResult result = vkAcquireNextImageKHR(
            m_Device->GetLogicalDevice(), m_SwapChain, UINT64_MAX,
            m_ImageAvailableSemaphores[m_CurrentFrameIndex], VK_NULL_HANDLE, &m_ImageIndex
        );

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            RecreateSwapChain();
            return;
        }
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        vkResetFences(m_Device->GetLogicalDevice(), 1, &m_InFlightFences[m_CurrentFrameIndex]);

        const VkCommandBuffer cmd = m_CommandBuffers[m_CurrentFrameIndex];
        vkResetCommandBuffer(cmd, 0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        RenderToScreen(cmd);

        if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }

        m_DrawQueue.clear();

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        const VkSemaphore waitSemaphores[] = {m_ImageAvailableSemaphores[m_CurrentFrameIndex]};
        constexpr VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;

        const VkSemaphore signalSemaphores[] = {m_RenderFinishedSemaphores[m_ImageIndex]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (
            vkQueueSubmit(
                m_Device->GetGraphicsQueue(),
                1,
                &submitInfo,
                m_InFlightFences[m_CurrentFrameIndex]
            ) != VK_SUCCESS
        ) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        const VkSwapchainKHR swapChains[] = {m_SwapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &m_ImageIndex;

        result = vkQueuePresentKHR(m_Device->GetPresentQueue(), &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_FramebufferResized) {
            RecreateSwapChain();
            m_FramebufferResized = false;
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
        m_TotalFramesRendered++;
    }

    void Renderer::UpdateCameraMatrix(const glm::mat4x4 &viewMatrix, const glm::mat4x4 &projectionMatrix) {
        UniformBufferObject ubo{};

        ubo.view = viewMatrix;
        ubo.proj = projectionMatrix;
        ubo.proj[1][1] *= -1;

        memcpy(m_UniformBuffersMapped[m_CurrentFrameIndex], &ubo, sizeof(UniformBufferObject));
    }

    void Renderer::SubmitDrawCall(
        const EntityID entityId,
        const glm::mat4x4 &worldMatrix,
        const uint32_t meshId,
        const uint32_t textureId
    ) {
        m_DrawQueue.push_back({
            worldMatrix,
            entityId,
            meshId,
            textureId
        });
    }

    void Renderer::CleanupSwapChain() const {
        m_Device->DestroyImageView(m_DepthImageView);
        m_Device->DestroyImage(m_DepthImage, m_DepthImageAllocation);

        for (const auto framebuffer: m_SwapChainFramebuffers) {
            m_Device->DestroyFramebuffer(framebuffer);
        }

        for (const auto imageView: m_SwapChainImageViews) {
            m_Device->DestroyImageView(imageView);
        }

        m_Device->DestroySwapchain(m_SwapChain);
    }

    void Renderer::Cleanup() {
        WaitIdle();

        // 1. Clear the frame-delayed resources (staging buffers, etc.)
        m_ResourceDeletionQueue->Flush(UINT32_MAX);

        // 2. Destroy objects managed by external managers
        if (GetTextureManager()) {
            GetTextureManager()->GraphicMemoryCleanup();
        }

        // 3. Destroy Global Textures and Buffers
        if (m_DefaultTextureImageView)
            m_Device->DestroyImageView(m_DefaultTextureImageView);
        if (m_DefaultTextureImage)
            m_Device->DestroyImage(
                m_DefaultTextureImage,
                m_DefaultTextureAllocation
            );

        if (m_VertexBuffer)
            m_Device->DestroyBuffer(
                m_VertexBuffer,
                m_VertexAllocation
            );

        if (m_IndexBuffer)
            m_Device->DestroyBuffer(
                m_IndexBuffer,
                m_IndexAllocation
            );

        // 4. Destroy Uniform Buffers (per frame)
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            m_Device->DestroyBuffer(m_UniformBuffers[i], m_UniformBuffersAllocations[i]);
        }

        // 5. Destroy Swapchain-related resources (Images are destroyed here via Swapchain)
        CleanupSwapChain();

        // 6. Destroy Sync Objects
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            m_Device->DestroySemaphore(m_ImageAvailableSemaphores[i]);
            m_Device->DestroyFence(m_InFlightFences[i]);
        }
        for (const auto renderFinishedSemaphore: m_RenderFinishedSemaphores) {
            m_Device->DestroySemaphore(renderFinishedSemaphore);
        }

        // 7. Destroy Pipeline and Layouts
        m_Device->DestroyPipeline(m_GraphicsPipeline);
        m_Device->DestroyPipelineLayout(m_PipelineLayout);
        m_Device->DestroyDescriptorSetLayout(m_DescriptorSetLayout);
        m_Device->DestroyDescriptorPool(m_DescriptorPool);
        m_Device->DestroySampler(m_TextureSampler);

        // 8. Destroy Command Pools and Render Pass
        m_Device->DestroyRenderPass(m_MainRenderPass);

        // DumpVmaStats();

        m_Device->Cleanup();
        m_Window->Destroy();
    }

    void Renderer::Reset() {
        GetMeshManager()->Reset();
        GetTextureManager()->Reset();
        m_DrawQueue.clear();
    }

    void Renderer::WaitIdle() const {
        m_Device->WaitIdle();
    }
}
