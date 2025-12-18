#ifndef GAME_ENGINE_COMPILE_H
#define GAME_ENGINE_COMPILE_H

#include <iostream>
#include <vector>
#include <shaderc/shaderc.hpp>

#include "types.h"

namespace Shaders {
    std::vector<uint32_t> Compile(
        const std::string &source_code,
        ShaderType type,
        const std::string &filename = "shader"
    );

    std::vector<uint32_t> CompileFromFile(
        const std::string &filepath,
        ShaderType type
    );
}

#endif //GAME_ENGINE_COMPILE_H
