#include "shader_module_cache.h"
#include "vulkan_device.h"
#include "../../logging/logger.h"
#include "../../utils/macros/log_macros.h"

namespace Vulkan {
    VkShaderModule ShaderModuleCache::CreateShaderModule(const std::vector<uint32_t> &code) const {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size() * sizeof(uint32_t);
        createInfo.pCode = code.data();

        return m_Device->CreateShaderModule(createInfo);
    }

    std::string ShaderModuleCache::GetShaderModuleKey(const std::string &path, const Shaders::ShaderType type) {
        return path + "_" + std::to_string(static_cast<int>(type));
    }

    ShaderModuleCache::ShaderModuleCache(const std::shared_ptr<VulkanDevice> &device) : m_Device(device) {
    }

    VkShaderModule ShaderModuleCache::GetOrCreateShaderModule(const std::string &path, const Shaders::ShaderType type) {
        const auto key = GetShaderModuleKey(path, type);

        if (m_Cache.contains(key)) {
            return m_Cache[key];
        }

        const auto shaderCode = Shaders::CompileFromFile(path, type);
        if (shaderCode.empty()) {
            throw std::runtime_error("Failed to compile shader: " + path);
        }

        const auto shaderModule = CreateShaderModule(shaderCode);
        m_Cache[key] = shaderModule;
        return shaderModule;
    }

    void ShaderModuleCache::DestroyShaderModule(const std::string &path, const Shaders::ShaderType type) {
        const auto key = GetShaderModuleKey(path, type);

        if (m_Cache.contains(key)) {
            m_Device->DestroyShaderModule(m_Cache[key]);
            m_Cache.erase(key);
        } else {
            LOG_WARN("Shader module was not deleted" + path);
        }
    }
} // Vulkan
