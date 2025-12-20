#ifndef GAME_ENGINE_VULKAN_TEXTURE_MANAGER_H
#define GAME_ENGINE_VULKAN_TEXTURE_MANAGER_H
#include <map>
#include <string>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "abstract_texture_manager.h"
#include "stb_image.h"

class AbstractRenderer;

namespace Vulkan {
    class Renderer;

    struct TextureInfo final : ITextureInfo {
        VkImage image = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        VkImageView imageView = VK_NULL_HANDLE;

        stbi_uc *pixels = nullptr;
        int width = 0;
        int height = 0;
        VkDeviceSize imageSize = 0;
    };

    class TextureManager final : public ITextureManager {
        AbstractRenderer *m_Renderer;
        std::map<TextureId, TextureInfo> m_TextureCatalog;
        TextureId m_NextTextureID = 0;

        TextureInfo m_DefaultTexture{};

    public:
        explicit TextureManager(AbstractRenderer *renderer);

        TextureId LoadTexture(TextureId textureId, const std::string &texturePath) override;

        TextureId LoadTexture(const std::string &texturePath) override;

        [[nodiscard]] VkImageView GetImageView(TextureId textureId) const;

        [[nodiscard]] VkSampler GetSampler() const;

        void DumpLoadedTextures(YAML::Emitter &out) const override;

        void GraphicMemoryCleanup() override;

        void Reset() override;

    private:
        void CreateTextureGPUResources(TextureId textureId);
    };
}


#endif //GAME_ENGINE_VULKAN_TEXTURE_MANAGER_H
