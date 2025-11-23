#include "compile.h"

#include <fstream>

#ifdef NDEBUG
constexpr shaderc_optimization_level optimizationLevel = shaderc_optimization_level_performance;
#else
constexpr shaderc_optimization_level optimizationLevel = shaderc_optimization_level_zero;
#endif

namespace Shaders {
    std::vector<uint32_t> Compile(
        const std::string &source_code,
        const shaderc_shader_kind shader_kind,
        const std::string &filename
    ) {
        const shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        options.SetOptimizationLevel(optimizationLevel);
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_0);

        const shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(
            source_code, shader_kind, filename.c_str(), options
        );

        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            std::cerr << "Shader compilation failed: " << result.GetErrorMessage() << std::endl;
            return {};
        }

        return {result.cbegin(), result.cend()};
    }

    std::vector<uint32_t> CompileFromFile(
        const std::string &filepath,
        const shaderc_shader_kind shader_kind
    ) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filepath << std::endl;
            return {};
        }

        const std::string source((std::istreambuf_iterator(file)),
                                 std::istreambuf_iterator<char>());

        return Compile(source, shader_kind, filepath);
    }
}
