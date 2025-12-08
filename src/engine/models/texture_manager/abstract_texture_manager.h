#ifndef VEE_ABSTRACT_TEXTURE_MANAGER_H
#define VEE_ABSTRACT_TEXTURE_MANAGER_H
#include <string>

#include "yaml-cpp/emitter.h"

using TextureId = std::uint32_t;

struct ITextureInfo {
    virtual ~ITextureInfo() = default;

    std::string path;
};

class ITextureManager {
public:
    virtual ~ITextureManager() = default;

    virtual TextureId LoadTexture(TextureId textureId, const std::string &texturePath) = 0;

    virtual TextureId LoadTexture(const std::string &texturePath) = 0;

    virtual void GraphicMemoryCleanup() = 0;

    virtual void DumpLoadedTextures(YAML::Emitter &out) const = 0;

    virtual void Reset() = 0;
};


#endif //VEE_ABSTRACT_TEXTURE_MANAGER_H
