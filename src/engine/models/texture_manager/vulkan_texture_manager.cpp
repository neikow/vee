#include "vulkan_texture_manager.h"

#include <ranges>

#include "../../renderer/vulkan/vulkan_renderer.h"

#include "stb_image.h"

namespace Vulkan {
    TextureInfo ParseTexture(const std::string &texturePath) {
        int texWidth, texHeight, texChannels;
        stbi_uc *pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        const VkDeviceSize imageSize = static_cast<VkDeviceSize>(texWidth) * texHeight * 4;

        if (!pixels) {
            throw std::runtime_error("Failed to load texture image from: " + texturePath);
        }

        TextureInfo textureInfo;
        textureInfo.path = texturePath;
        textureInfo.pixels = pixels;
        textureInfo.width = texWidth;
        textureInfo.height = texHeight;
        textureInfo.imageSize = imageSize;

        return textureInfo;
    }

    TextureId TextureManager::LoadTexture(const std::string &texturePath) {
        const TextureId newID = m_NextTextureID++;
        m_TextureCatalog[newID] = ParseTexture(texturePath);

        CreateTextureGPUResources(newID);

        return newID;
    }

    TextureManager::TextureManager(AbstractRenderer *renderer) : m_Renderer(renderer) {
    }

    TextureId TextureManager::LoadTexture(const TextureId textureId, const std::string &texturePath) {
        m_TextureCatalog[textureId] = ParseTexture(texturePath);

        m_Renderer->EnqueuePostInitTask([this, textureId] {
            CreateTextureGPUResources(textureId);
        });

        return textureId;
    }

    void TextureManager::GraphicMemoryCleanup() {
        m_TextureCatalog.clear();
    }

    void TextureManager::Reset() {
        GraphicMemoryCleanup();
        m_NextTextureID = 0;
    }

    void TextureManager::CreateTextureGPUResources(const TextureId textureId) {
        auto &info = m_TextureCatalog[textureId];
        const auto renderer = dynamic_cast<Renderer *>(m_Renderer);

        if (!renderer->Initialized()) {
            return;
        }

        if (info.image != VK_NULL_HANDLE) {
            vkDestroyImageView(renderer->m_Device, info.imageView, nullptr);
            vkDestroyImage(renderer->m_Device, info.image, nullptr);
            vkFreeMemory(renderer->m_Device, info.imageMemory, nullptr);
            info.image = VK_NULL_HANDLE;
        }

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

        renderer->UpdateTextureDescriptor(textureId);

        stbi_image_free(info.pixels);
        info.pixels = nullptr;

        renderer->EnqueueCleanupTask([this, renderer, textureId] {
            const auto textureInfo = m_TextureCatalog[textureId];
            vkDestroyImage(renderer->m_Device, textureInfo.image, nullptr);
            vkFreeMemory(renderer->m_Device, textureInfo.imageMemory, nullptr);
            vkDestroyImageView(renderer->m_Device, textureInfo.imageView, nullptr);
        });
    }

    VkImageView TextureManager::GetImageView(const TextureId textureId) const {
        if (m_TextureCatalog.contains(textureId)) {
            return m_TextureCatalog.at(textureId).imageView;
        }

        return nullptr;
    }

    VkSampler TextureManager::GetSampler() const {
        const auto renderer = dynamic_cast<Renderer *>(m_Renderer);

        return renderer->m_TextureSampler;
    }

    void TextureManager::DumpLoadedTextures(YAML::Emitter &out) const {
        out << YAML::Key << "textures" << YAML::Value << YAML::BeginSeq;
        for (const auto &[textureId, textureInfo]: m_TextureCatalog) {
            out << YAML::BeginMap;
            out << YAML::Key << "id" << YAML::Value << textureId;
            out << YAML::Key << "path" << YAML::Value << textureInfo.path;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    }
}
