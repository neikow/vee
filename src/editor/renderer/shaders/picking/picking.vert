#version 450

layout(push_constant) uniform PushConstants {
    mat4 model;
    uint textureId;
    uint entityId;
} pcs;

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;

void main() {
    gl_Position = ubo.proj * ubo.view * pcs.model * vec4(inPosition, 1.0);
}