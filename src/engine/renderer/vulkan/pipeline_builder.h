#ifndef VEE_PIPELINE_BUILDER_H
#define VEE_PIPELINE_BUILDER_H

#include <cstring>
#include <functional>
#include <vulkan/vulkan.h>

template<>
struct std::hash<VkExtent2D> {
    size_t operator()(VkExtent2D const &extent) const noexcept {
        return std::hash<uint32_t>()(extent.width) ^ (std::hash<uint32_t>()(extent.height) << 1);
    }
};

namespace Vulkan {
    class VulkanDevice;

    enum VertexLayout {
        DEFAULT,
        POSITION_ONLY,
    };

#pragma pack(push, 1)
    struct PipelineStateKey {
        // Shaders
        VkShaderModule vertexShader;
        VkShaderModule fragmentShader;

        // Fixed-function states
        VkPrimitiveTopology topology;

        // Rasterization
        VkPolygonMode polygonMode;
        VkCullModeFlags cullMode;
        VkFrontFace frontFace;

        // Color Blending
        VkBool32 blendEnable;
        VkBlendFactor srcColorBlendFactor;
        VkBlendFactor dstColorBlendFactor;
        VkBlendOp colorBlendOp;
        VkBlendFactor srcAlphaBlendFactor;
        VkBlendFactor dstAlphaBlendFactor;
        VkBlendOp alphaBlendOp;
        VkColorComponentFlags colorWriteMask;

        // Depth/Stencil
        VkBool32 depthTest;
        VkBool32 depthWrite;
        VkCompareOp depthCompareOp;

        // Layouts
        VertexLayout vertexLayout;

        // Viewport
        VkExtent2D viewportExtent;

        // Compatibility
        VkRenderPass renderPass;
        uint32_t subpass;

        bool operator==(const PipelineStateKey &other) const {
            return memcmp(
                       this,
                       &other,
                       sizeof(PipelineStateKey)
                   ) == 0;
        }
    };
#pragma pack(pop)

    struct PipelineStateHash {
        std::size_t operator()(const PipelineStateKey &key) const {
            std::size_t seed = 0;

            auto hash_combine = [&seed]<typename Type>(Type val) {
                std::hash<Type> hasher;
                seed ^= hasher(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            };

            hash_combine(key.vertexShader);
            hash_combine(key.fragmentShader);

            hash_combine(key.topology);
            hash_combine(key.cullMode);
            hash_combine(key.frontFace);
            hash_combine(key.polygonMode);

            hash_combine(key.depthTest);
            hash_combine(key.depthWrite);
            hash_combine(key.depthCompareOp);

            hash_combine(key.vertexLayout);

            hash_combine(key.blendEnable);
            hash_combine(key.srcColorBlendFactor);
            hash_combine(key.dstColorBlendFactor);
            hash_combine(key.colorBlendOp);
            hash_combine(key.srcAlphaBlendFactor);
            hash_combine(key.dstAlphaBlendFactor);
            hash_combine(key.alphaBlendOp);
            hash_combine(key.colorWriteMask);

            hash_combine(key.viewportExtent);

            hash_combine(key.renderPass);
            hash_combine(key.subpass);

            return seed;
        }
    };

    class PipelineCache {
        std::unordered_map<PipelineStateKey, VkPipeline, PipelineStateHash> m_Cache;

        static VkPipeline CreateNewVulkanPipeline(
            const PipelineStateKey &key,
            const std::shared_ptr<VulkanDevice> &device,
            const VkPipelineLayout &layout
        );

    public:
        VkPipeline GetOrCreatePipeline(
            const PipelineStateKey &key,
            const std::shared_ptr<VulkanDevice> &device,
            const VkPipelineLayout &layout
        );
    };

    class PipelineBuilder {
    public:
        PipelineStateKey m_Key{};

        PipelineBuilder &SetShaders(
            const VkShaderModule &vertexShader,
            const VkShaderModule &fragmentShader
        );

        PipelineBuilder &SetTopology(
            VkPrimitiveTopology topology
        );

        PipelineBuilder &SetDepthState(
            VkBool32 test,
            VkBool32 write,
            VkCompareOp op
        );

        PipelineBuilder &SetRenderPass(
            const VkRenderPass &rp
        );

        PipelineBuilder &SetExtent(
            const VkExtent2D &extent
        );

        PipelineBuilder &SetVertexLayout(
            VertexLayout layout
        );

        PipelineBuilder &SetRasterizer(
            const VkPolygonMode &polygonMode,
            const VkCullModeFlags &cullMode,
            const VkFrontFace &frontFace
        );

        PipelineBuilder &SetColorBlending(
            VkBool32 blendEnable,
            const VkBlendFactor &srcColorBlendFactor,
            const VkBlendFactor &dstColorBlendFactor,
            const VkBlendOp &colorBlendOp,
            const VkBlendFactor &srcAlphaBlendFactor,
            const VkBlendFactor &dstAlphaBlendFactor,
            const VkBlendOp &alphaBlendOp,
            const VkColorComponentFlags &colorWriteMask
        );

        [[nodiscard]] VkPipeline Build(
            const std::shared_ptr<VulkanDevice> &device,
            const VkPipelineLayout &layout,
            const std::shared_ptr<PipelineCache> &cache
        ) const;
    };
}

#endif
