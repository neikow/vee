#ifndef GAME_ENGINE_COMPILE_H
#define GAME_ENGINE_COMPILE_H

#include <iostream>
#include <vector>
#include <shaderc/shaderc.hpp>

namespace Shaders {
    std::vector<uint32_t> Compile(
        const std::string &source_code,
        shaderc_shader_kind shader_kind,
        const std::string &filename = "shader"
    );

    std::vector<uint32_t> CompileFromFile(
        const std::string &filepath,
        shaderc_shader_kind shader_kind
    );
}

#endif //GAME_ENGINE_COMPILE_H
