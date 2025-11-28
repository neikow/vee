#ifndef VEE_ABSTRACT_TEXTURE_MANAGER_H
#define VEE_ABSTRACT_TEXTURE_MANAGER_H
#include <string>

using TextureId = std::uint32_t;

class ITextureManager {
public:
    virtual ~ITextureManager() = default;

    virtual TextureId LoadTexture(TextureId textureId, const std::string &texturePath) = 0;

    virtual TextureId LoadTexture(const std::string &texturePath) = 0;

    virtual void Cleanup() = 0;

    virtual void Reset() = 0;
};


#endif //VEE_ABSTRACT_TEXTURE_MANAGER_H
