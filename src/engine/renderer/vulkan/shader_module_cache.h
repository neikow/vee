#ifndef VEE_SHADER_MODULE_CACHE_H
#define VEE_SHADER_MODULE_CACHE_H
#include <map>
#include <string>

#include <vulkan/vulkan.h>
#include "../../shaders/compile.h"
#include "../../shaders/types.h"

namespace Vulkan {
    class VulkanDevice;

    /** Cache for Vulkan shader modules to avoid redundant creations. */
    class ShaderModuleCache {
        std::map<std::string, VkShaderModule> m_Cache;
        std::shared_ptr<VulkanDevice> m_Device;

        /** Create a Vulkan shader module from the given SPIR-V code.
         *
         * @param code The SPIR-V bytecode of the shader.
         * @return The created VkShaderModule.
         */
        VkShaderModule CreateShaderModule(const std::vector<uint32_t> &code) const;

        /** Generate a unique key for the shader module based on its path and type.
         *
         * @param path The file path to the shader source code.
         * @param type The type of the shader (vertex, fragment, etc.).
         * @return A unique string key for the shader module.
         */
        static inline std::string GetShaderModuleKey(const std::string &path, Shaders::ShaderType type);

    public:
        explicit ShaderModuleCache(const std::shared_ptr<VulkanDevice> &device);;

        /** Get or create a shader module from the given path and type.
         * If the shader module already exists in the cache, it is returned.
         * Otherwise, it is created, cached, and then returned.
         *
         * @param path The file path to the shader source code.
         * @param type The type of the shader (vertex, fragment, etc.).
         * @return The VkShaderModule corresponding to the shader.
         */
        VkShaderModule GetOrCreateShaderModule(
            const std::string &path,
            Shaders::ShaderType type
        );

        /** Destroy the shader module associated with the given path and type.
         *
         * @param path The file path to the shader source code.
         * @param type The type of the shader (vertex, fragment, etc.).
         */
        void DestroyShaderModule(const std::string &path, Shaders::ShaderType type);
    };
} // Vulkan

#endif //VEE_SHADER_MODULE_CACHE_H
