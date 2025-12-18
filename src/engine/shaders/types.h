#ifndef VEE_TYPES_H
#define VEE_TYPES_H
#include <shaderc/shaderc.h>

namespace Shaders {
    enum class ShaderType {
        Vertex = shaderc_vertex_shader,
        Fragment = shaderc_fragment_shader,
        Compute = shaderc_compute_shader,
    };
}

#endif //VEE_TYPES_H
