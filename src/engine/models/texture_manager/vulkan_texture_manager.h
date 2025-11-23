#ifndef GAME_ENGINE_VULKAN_TEXTURE_MANAGER_H
#define GAME_ENGINE_VULKAN_TEXTURE_MANAGER_H
#include <map>
#include <string>
#include <vulkan/vulkan.h>

#include "stb_image.h"

namespace Vulkan {
    class Renderer;
    using TextureId = uint32_t;

    struct TextureInfo {
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory imageMemory = VK_NULL_HANDLE;
        VkImageView imageView = VK_NULL_HANDLE;

        stbi_uc *pixels = nullptr;
        int width = 0;
        int height = 0;
        VkDeviceSize imageSize = 0;
    };

    class TextureManager {
        std::shared_ptr<Renderer> m_Renderer = nullptr;
        std::map<TextureId, TextureInfo> m_TextureCatalog;
        TextureId m_NextTextureID = 0;

        TextureInfo m_DefaultTexture{};

    public:
        TextureManager() = default;

        void BindRenderer(const std::shared_ptr<Renderer> &renderer) {
            m_Renderer = renderer;
        }

        TextureId LoadTexture(const std::string &texturePath);

        void CreateResources();

        VkImageView GetImageView(TextureId textureId) const;

        VkSampler GetSampler() const;

        void Cleanup();

    private:
        static void CreateDefaultTexture(const Renderer *renderer, TextureInfo &outInfo);
    };
}


#endif //GAME_ENGINE_VULKAN_TEXTURE_MANAGER_H
