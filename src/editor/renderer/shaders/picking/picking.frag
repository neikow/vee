#version 450

layout(location = 0) out uint outColor;

layout(push_constant) uniform PushConstants {
    mat4 model;
    uint textureId;
    uint entityId;
} pcs;

const uint RESERVER_ID = 0u;

void main() {
    outColor = pcs.entityId;
}