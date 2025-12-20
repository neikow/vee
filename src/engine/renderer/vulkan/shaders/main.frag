#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstants {
    mat4 model;
    uint textureId;
} pcs;

layout (set = 0, binding = 0) uniform sampler textureSampler;
layout (set = 0, binding = 1) uniform texture2D textures[];

void main() {
    outColor = texture(sampler2D(textures[pcs.textureId], textureSampler), fragTexCoord * 1.0f);
}