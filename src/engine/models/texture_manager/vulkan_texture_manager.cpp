#include "vulkan_texture_manager.h"

#include <ranges>

#include "../../renderer/vulkan/vulkan_renderer.h"

#include "stb_image.h"

namespace Vulkan {
    static constexpr uint8_t DEFAULT_WHITE_PIXEL[4] = {255, 255, 255, 255};

    TextureInfo ParseTexture(const std::string &texturePath) {
        int texWidth, texHeight, texChannels;
        stbi_uc *pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        const VkDeviceSize imageSize = static_cast<VkDeviceSize>(texWidth) * texHeight * 4;

        if (!pixels) {
            throw std::runtime_error("Failed to load texture image from: " + texturePath);
        }

        return {
            .pixels = pixels,
            .width = texWidth,
            .height = texHeight,
            .imageSize = imageSize
        };
    }

    TextureId TextureManager::LoadTexture(const std::string &texturePath) {
        const TextureId newID = m_NextTextureID++;
        m_TextureCatalog[newID] = ParseTexture(texturePath);

        return newID;
    }

    TextureId TextureManager::LoadTexture(const TextureId textureId, const std::string &texturePath) {
        m_TextureCatalog[textureId] = ParseTexture(texturePath);

        return textureId;
    }

    void TextureManager::CreateDefaultTexture(const Renderer *renderer, TextureInfo &outInfo) {
        constexpr VkDeviceSize imageSize = 4;
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        renderer->CreateBuffer(
            imageSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingBufferMemory,
            {}
        );

        void *data;
        vkMapMemory(renderer->m_Device, stagingBufferMemory, 0, imageSize, 0, &data);
        std::memcpy(data, DEFAULT_WHITE_PIXEL, static_cast<size_t>(imageSize));
        vkUnmapMemory(renderer->m_Device, stagingBufferMemory);

        renderer->CreateImage(
            1, 1, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, outInfo.image, outInfo.imageMemory, {}
        );

        renderer->TransitionImageLayout(
            outInfo.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        );
        renderer->CopyBufferToImage(
            stagingBuffer, outInfo.image, 1, 1
        );
        renderer->TransitionImageLayout(
            outInfo.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        outInfo.imageView = renderer->
                CreateImageView(outInfo.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

        vkDestroyBuffer(renderer->m_Device, stagingBuffer, nullptr);
        vkFreeMemory(renderer->m_Device, stagingBufferMemory, nullptr);

        outInfo.width = 1;
        outInfo.height = 1;
        outInfo.imageSize = imageSize;
    }

    void TextureManager::CreateResources() {
        const auto renderer = dynamic_cast<Renderer *>(m_Renderer);

        if (!renderer->m_Device) {
            throw std::runtime_error(
                "Cannot create Vulkan resources: Device not intialized. Please initialize the Renderer before calling this method."
            );
        }

        if (m_DefaultTexture.imageView == VK_NULL_HANDLE) {
            CreateDefaultTexture(renderer, m_DefaultTexture);
        }

        for (auto &info: m_TextureCatalog | std::views::values) {
            if (info.image != VK_NULL_HANDLE) continue;

            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;

            renderer->CreateBuffer(
                info.imageSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                stagingBuffer,
                stagingBufferMemory,
                {}
            );

            void *data;
            vkMapMemory(renderer->m_Device, stagingBufferMemory, 0, info.imageSize, 0, &data);
            memcpy(data, info.pixels, info.imageSize);
            vkUnmapMemory(renderer->m_Device, stagingBufferMemory);

            renderer->CreateImage(
                info.width, info.height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, info.image, info.imageMemory, {}
            );

            renderer->TransitionImageLayout(
                info.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
            );
            renderer->CopyBufferToImage(
                stagingBuffer, info.image, static_cast<uint32_t>(info.width), static_cast<uint32_t>(info.height)
            );
            renderer->TransitionImageLayout(
                info.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            );

            info.imageView = renderer->CreateImageView(
                info.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT
            );

            vkDestroyBuffer(renderer->m_Device, stagingBuffer, nullptr);
            vkFreeMemory(renderer->m_Device, stagingBufferMemory, nullptr);

            stbi_image_free(info.pixels);
            info.pixels = nullptr;
        }
    }

    void TextureManager::Cleanup() {
        const auto renderer = dynamic_cast<Renderer *>(m_Renderer);

        for (const auto &info: m_TextureCatalog | std::views::values) {
            vkDestroyImageView(renderer->m_Device, info.imageView, nullptr);
            vkDestroyImage(renderer->m_Device, info.image, nullptr);
            vkFreeMemory(renderer->m_Device, info.imageMemory, nullptr);
        }

        m_TextureCatalog.clear();

        if (m_DefaultTexture.imageView != VK_NULL_HANDLE) {
            vkDestroyImageView(renderer->m_Device, m_DefaultTexture.imageView, nullptr);
            m_DefaultTexture.imageView = VK_NULL_HANDLE;
        }
        if (m_DefaultTexture.image != VK_NULL_HANDLE) {
            vkDestroyImage(renderer->m_Device, m_DefaultTexture.image, nullptr);
            m_DefaultTexture.image = VK_NULL_HANDLE;
        }
        if (m_DefaultTexture.imageMemory != VK_NULL_HANDLE) {
            vkFreeMemory(renderer->m_Device, m_DefaultTexture.imageMemory, nullptr);
            m_DefaultTexture.imageMemory = VK_NULL_HANDLE;
        }
    }

    void TextureManager::Reset() {
        const auto renderer = dynamic_cast<Renderer *>(m_Renderer);

        Cleanup();
        m_NextTextureID = 0;
        renderer->CreateDescriptorSets();
    }

    VkImageView TextureManager::GetImageView(const TextureId textureId) const {
        if (m_TextureCatalog.contains(textureId)) {
            return m_TextureCatalog.at(textureId).imageView;
        }

        return m_DefaultTexture.imageView;
    }

    VkSampler TextureManager::GetSampler() const {
        const auto renderer = dynamic_cast<Renderer *>(m_Renderer);

        return renderer->m_TextureSampler;
    }
}
